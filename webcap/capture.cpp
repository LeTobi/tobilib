#include "capture.h"
#include "error.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace tobilib
{
	void Capture::open(const StringPlus& dev)
	{
		if (device!=-1)
		{
			if (dev != devname)
				throw capture_error("Es ist bereits ein anderes Geraet offen");
			return;
		}
		devname = dev;
		device = ::open(dev.toString().c_str(),O_RDWR);
		if (device == -1)
			throw capture_error((dev+" konnte nicht geoeffnet werden.").toString());
		_status = State::open;
	}
	
	StringPlus Capture::check()
	{
		if (_status != State::open)
			throw capture_error("Im aktuellen Status kann kein Kameracheck gemacht werden.");
		StringPlus out;
		v4l2_capability info;
		if (ioctl(device,VIDIOC_QUERYCAP,&info)==-1)
			throw capture_error(std::string("Fehler bei der Abfrage von Kameraeigenschaften.\nFehlercode: ")+std::to_string(errno));
		out = StringPlus("Kameradaten gefunden\nName: ")+(char*)info.card+"\nDriver: "+(char*)info.driver;
		int caps = info.device_caps;
		if (!(info.capabilities & 0x80000000))
		{
			// info: Ein gerät kann auf mehrere Dateien aufgeteilt sein.
			// Idealerweise werden die Fähigkeiten der Datei und nicht des Geräts angegeben.
			out += "\nWarnung: Es wurden keine Dateispezifischen Capabilities in VIDIOC_QUERYCAP angegeben!";
			caps = info.capabilities;
		}
		if (!(caps & 0x00000001))
			throw capture_error("Das Geraet kann keine Videoaufnahmen machen.");
		if (!(caps & 0x04000000))
			throw capture_error("Das Geraet unterstuetzt kein Videostreaming.");
		return out;
	}
	
	bool Capture::check_format(const v4l2_format& format)
	{
		return
			format.type == V4L2_BUF_TYPE_VIDEO_CAPTURE &&
			format.fmt.pix.pixelformat == v4l2_fourcc('Y','U','Y','V');
	}
	
	void Capture::streamon()
	{
		if (_status == State::streaming)
			return;
		if (_status == State::closed)
			throw capture_error("Die Kamera ist noch nicht offen. Streaming nicht moeglich.");
		v4l2_format format;
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(device,VIDIOC_G_FMT,&format) == -1)
			throw capture_error(std::string("Fehler bei VIDIOC_G_FMT\nFehlercode: ")+std::to_string(errno));
		if (!check_format(format))
		{
			format.fmt.pix.pixelformat = v4l2_fourcc('Y','U','Y','V');
			if (ioctl(device,VIDIOC_S_FMT,&format) == -1)
				throw capture_error(std::string("Fehler bei VIDIOC_S_FMT\nFehlercode: ")+std::to_string(errno));
		}
		if (!check_format(format))
			throw capture_error("Der Treiber unterstuetzt das YUYV-Format nicht");
		imgwidth = format.fmt.pix.width;
		imgheight = format.fmt.pix.height;
		v4l2_requestbuffers reqbuf;
		reqbuf.count = 1;
		reqbuf.type = format.type;
		reqbuf.memory = V4L2_MEMORY_MMAP;
		if (ioctl(device,VIDIOC_REQBUFS,&reqbuf) == -1)
			throw capture_error(std::string("Fehler beim Anfordern der Videobuffer\nFehlercode: ")+std::to_string(errno));
		bufferinfo.index = 0;
		bufferinfo.type = format.type;
		if (ioctl(device,VIDIOC_QUERYBUF,&bufferinfo) == -1)
			throw capture_error(std::string("Fehler bei Abfrage des Buffers\nFehlercode: ")+std::to_string(errno));
		// Falls der folgende Fehler auftaucht, muss die Implementierung angepasst werden:
		if (bufferinfo.length != format.fmt.pix.sizeimage)
			throw capture_error("Der Buffer passt nicht zu den Bilddimensionen.");
		cambuffer.map(device,bufferinfo.m.offset,bufferinfo.length);
		rawdata.create(cambuffer.length()/2*3);
		if (ioctl(device,VIDIOC_STREAMON,&format.type) == -1)
			throw capture_error(std::string("Fehler beim Start des Streams\nFehlercode: ")+std::to_string(errno));
		_status = State::streaming;
	}
	
	void Capture::read()
	{
		if (_status!=State::streaming)
			throw capture_error("Fuer einen Schnappschuss muss ein Streaming begonnen werden.");
		if (ioctl(device,VIDIOC_QBUF,&bufferinfo))
			throw capture_error(std::string("Fehler bei Einreihen des Buffers\nFehlercode: ")+std::to_string(errno));
		if (ioctl(device,VIDIOC_DQBUF,&bufferinfo))
			throw capture_error(std::string("Fehler beim Auslesen der Kameradaten\nFehlercode: ")+std::to_string(errno));
		// Falls der folgende Fehler auftritt, muss die Implementierung angepasst werden:
		if (bufferinfo.bytesused < cambuffer.length())
			throw capture_error("Ein Buffer wurde nicht komplett gefuellt");
		cambuffer.used = bufferinfo.bytesused;
		for (int i=0;i<cambuffer.length()/2;i+=2)
		{
			rawdata.start()[3*i + 0] = cambuffer.start()[2*i + 0];
			rawdata.start()[3*i + 1] = cambuffer.start()[2*i + 1];
			rawdata.start()[3*i + 2] = cambuffer.start()[2*i + 3];
			rawdata.start()[3*i + 3] = cambuffer.start()[2*i + 2];
			rawdata.start()[3*i + 4] = cambuffer.start()[2*i + 1];
			rawdata.start()[3*i + 5] = cambuffer.start()[2*i + 3];
		}
		rawdata.used = rawdata.length();
		try {
			encoder.encode_yuv(rawdata,imgwidth,imgheight);
		} catch (jpeg_error& e) {
			throw capture_error(std::string("Fehler bei JPEG-Kompression: ")+e.what());
		}
	}
	
	void Capture::streamoff()
	{
		if (device==-1)
			return;
		rawdata.clear();
		cambuffer.clear();
		if (_status != State::streaming)
			return;
		int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ioctl(device,VIDIOC_STREAMOFF,&type);
		_status = State::open;
	}
	
	void Capture::close()
	{
		if (device == -1)
			return;
		streamoff();
		::close(device);
		device = -1;
		_status = State::closed;
	}
}