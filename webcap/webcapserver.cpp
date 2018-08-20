#include "webcapserver.h"
#include <boost/bind.hpp>
#include <ctime>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h> // mmap
#include <cstdio>
#include <jpeglib.h>

template <typename T>
inline void debug_log(T txt)
{
	/*
	std::cout << "Debug: " << txt << std::endl;
	//*/
}

using namespace webcap;
using namespace std;

void Buffer::allocate(int len)
{
	debug_log(string("Buffer::allocate(")+to_string(len)+")");
	clear();
	type = ptr_type::memory;
	_length = len;
	_start = new char [len];
	used = 0;
}

void Buffer::map(int device, int offset, int len)
{
	debug_log(string("Buffer::map() len=")+to_string(len));
	clear();
	_start = static_cast<char*>(mmap(NULL,len,PROT_READ | PROT_WRITE, MAP_SHARED, device, offset));
	if (_start == NULL)
		throw cam_error("Fehler beim Mapping von Buffer");
	type = ptr_type::map;
	_length = len;
	used = 0;
}

void Buffer::clear()
{
	debug_log(string("Buffer::clear() len=")+to_string(_length));
	if (_start == NULL)
		return;
	if (type == ptr_type::memory)
	{
		delete _start;
		_start = NULL;
	}
	else if (type == ptr_type::map)
	{
		munmap(_start,_length);
		_start = NULL;
	}
	_length = 0;
	used = 0;
}

Buffer::~Buffer()
{
	clear();
}

void Capture::open(string p)
{
	debug_log(string("Open device ")+p);
	if (p.size()>0)
		path = p;
	if (device == -1)
		device = ::open(path.c_str(),O_RDWR);
	if (device == -1)
		throw cam_error(string("Es konnte nicht auf die Webcam zugegriffen werden:\n")+path+"\nFehlercode: "+to_string(errno));
}

string Capture::check()
{
	debug_log("Capture::check()");
	open();
	string out;
	v4l2_capability info;
	if (ioctl(device,VIDIOC_QUERYCAP,&info)==-1)
		throw cam_error(string("Fehler bei der Abfrage von Kameraeigenschaften.\nFehlercode: ")+to_string(errno));
	out = string("Kameradaten gefunden\nName: ")+(char*)info.card+"\nDriver: "+(char*)info.driver;
	int caps = info.device_caps;
	if (!(info.capabilities & 0x80000000))
	{
		out += "\nWarnung: Es wurden keine Dateispezifischen Capabilities in VIDIOC_QUERYCAP angegeben!";
		caps = info.capabilities;
	}
	if (!(caps & 0x00000001))
		throw cam_error("Das Geraet kann keine Videoaufnahmen machen.");
	if (!(caps & 0x04000000))
		throw cam_error("Das Geraet unterstuetzt kein Videostreaming.");
	close();
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
	debug_log("Capture::streamon()");
	open();
	v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(device,VIDIOC_G_FMT,&format) == -1)
		throw cam_error(string("Fehler bei VIDIOC_G_FMT\nFehlercode: ")+to_string(errno));
	if (!check_format(format))
	{
		format.fmt.pix.pixelformat = v4l2_fourcc('Y','U','Y','V');
		if (ioctl(device,VIDIOC_S_FMT,&format) == -1)
			throw cam_error(string("Fehler bei VIDIOC_S_FMT\nFehlercode: ")+to_string(errno));
	}
	if (!check_format(format))
		throw cam_error("Der Treiber unterstuetzt das YUYV-Format nicht");
	imgwidth = format.fmt.pix.width;
	imgheight = format.fmt.pix.height;
	v4l2_requestbuffers reqbuf;
	reqbuf.count = 1;
	reqbuf.type = format.type;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (ioctl(device,VIDIOC_REQBUFS,&reqbuf) == -1)
		throw cam_error(string("Fehler beim Anfordern der Videobuffer\nFehlercode: ")+to_string(errno));
	bufferinfo.index = 0;
	bufferinfo.type = format.type;
	if (ioctl(device,VIDIOC_QUERYBUF,&bufferinfo) == -1)
		throw cam_error(string("Fehler bei Abfrage des Buffers\nFehlercode: ")+to_string(errno));
	if (bufferinfo.length != format.fmt.pix.sizeimage)
		throw cam_error("Der Buffer passt nicht zu den Bilddimensionen.");
	cam_data.map(device,bufferinfo.m.offset,bufferinfo.length);
	raw_data.allocate(cam_data.length()/2*3);
	message_data.allocate(raw_data.length());
	if (ioctl(device,VIDIOC_STREAMON,&format.type) == -1)
		throw cam_error(string("Fehler beim Start des Streams\nFehlercode: ")+to_string(errno));
	streaming = true;
}

void Capture::read()
{
	debug_log("Capture::read()");
	if (!streaming)
		throw cam_error("Es wurde versucht Daten ohne laufenden Stream auszulesen.");
	if (ioctl(device,VIDIOC_QBUF,&bufferinfo))
		throw cam_error(string("Fehler bei Einreihen des Buffers\nFehlercode: ")+to_string(errno));
	if (ioctl(device,VIDIOC_DQBUF,&bufferinfo))
		throw cam_error(string("Fehler beim Auslesen der Kameradaten\nFehlercode: ")+to_string(errno));
	if (bufferinfo.bytesused < cam_data.length())
		throw cam_error("Ein Buffer wurde nicht komplett gefuellt");
	debug_log("Cam Data read");
	for (int i=0;i<cam_data.length()/2;i+=2)
	{
		raw_data.start()[3*i + 0] = cam_data.start()[2*i + 0];
		raw_data.start()[3*i + 1] = cam_data.start()[2*i + 1];
		raw_data.start()[3*i + 2] = cam_data.start()[2*i + 3];
		raw_data.start()[3*i + 3] = cam_data.start()[2*i + 2];
		raw_data.start()[3*i + 4] = cam_data.start()[2*i + 1];
		raw_data.start()[3*i + 5] = cam_data.start()[2*i + 3];
	}
	debug_log("Buffer rearranged");
	jpeg_compress_struct compression;
	jpeg_error_mgr error;
	compression.err = jpeg_std_error(&error);
	jpeg_create_compress(&compression);
	FILE * outf = fmemopen(message_data.start()+1,message_data.length()-1,"w");
	if (outf==NULL)
		throw cam_error("Fehler beim erstellen des Komprimier-Buffers");
	jpeg_stdio_dest(&compression,outf);
	compression.image_width = imgwidth;
	compression.image_height = imgheight;
	compression.input_components = 3;
	compression.in_color_space = JCS_YCbCr;
	jpeg_set_defaults(&compression);
	debug_log("Compression set up");
	jpeg_start_compress(&compression,true);
	for (int i=0;i<raw_data.length();i+=3*imgwidth)
	{
		unsigned char * offset = reinterpret_cast<unsigned char*>(raw_data.start()+i);
		jpeg_write_scanlines(&compression,&offset,1);
	}
	debug_log("Lines fed");
	jpeg_finish_compress(&compression);
	debug_log("Compression done");
	message_data.used = ftell(outf);
	fclose(outf);
	jpeg_destroy_compress(&compression);
	*message_data.start() = (char)message_data.used;
	debug_log("Message ready");
}

void Capture::streamoff()
{
	debug_log("Capture::streamoff()");
	raw_data.clear();
	cam_data.clear();
	message_data.clear();
	if (!streaming)
		return;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(device,VIDIOC_STREAMOFF,&type);
	streaming = false;
}

void Capture::close()
{
	debug_log("Capture::close()");
	if (device == -1)
		return;
	streamoff();
	::close(device);
	device = -1;
}

void Session::start()
{
	debug_log(string("Session::start() ")+ip.to_string());
	boost::system::error_code err;
	socket.accept(err);
	if (err)
		close();
	socket.binary(true);
}

void Session::transmit(Buffer& data)
{
	debug_log(string("Session::transmit() len=")+to_string(data.used)+" "+ip.to_string());
	boost::system::error_code err;
	socket.write(boost::asio::buffer(data.start(),data.used),err);
	if (err)
		close();
}

void Session::close()
{
	debug_log(string("Session::close() ")+ip.to_string());
	boost::system::error_code err;
	socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both,err);
	socket.next_layer().close(err);
	alive=false;
}

void Streamserver::writelog(const char * txt)
{
	time_t rawtime;
	time(&rawtime);
	tm* tinfo = localtime(&rawtime);
	char tbuff [20];
	strftime(tbuff,20,"%Y %m %d %T",tinfo);
	loglock.lock();
	*log << tbuff << endl << "\t";
	int i=0;
	while (txt[i]!='\0')
	{
		if (txt[i] == '\n')
			*log << "\n\t";
		else
			*log << txt[i];
		i++;
	}
	*log << endl;
	log->flush();
	loglock.unlock();
}

void Streamserver::writelog(string str)
{
	writelog(str.c_str());
}

void Streamserver::allow_ip(boost::asio::ip::address ip)
{
	if (!is_allowed(ip))
	{
		listlock.lock();
		whitelist.push_back(ip);
		writelog(string("Whitelist wurde erweitert: ")+ip.to_string());
		listlock.unlock();
	}
	
}

void Streamserver::reject_ip(boost::asio::ip::address ip)
{
	listlock.lock();
	for (auto it = whitelist.begin();it!=whitelist.end();it++)
	{
		if (*it == ip)
		{
			writelog(string("Adresse wird aus Whitelist entfernt: ")+ip.to_string());
			whitelist.erase(it);
			break;
		}
	}
	listlock.unlock();
}

bool Streamserver::active()
{
	return running;
}

int Streamserver::client_max(int m)
{
	if (m>0)
	{
		maxsessions = m;
		writelog(string("Die maximale Client-Anzahl wurde angepasst: ")+to_string(maxsessions));
	}
	return maxsessions;
}

int Streamserver::client_count()
{
	int out = 0;
	listlock.lock();
	out = sessions.size();
	listlock.unlock();
	return out;
}

void Streamserver::logstream(ostream* ostr)
{
	loglock.lock();
	log = ostr;
	loglock.unlock();
}

void Streamserver::run(string path)
{
	if (!ready)
	{
		writelog("Der Server ist nicht bereit fuer einen Neustart");
		return;
	}
	writelog("Der server wird gestartet.");
	if (maxsessions<1)
	{
		writelog("Es muss mindestens eine Verbindung zugelassen werden.");
		return;
	}
	try
	{
		cam.open(path);
		writelog(cam.check());
		cam.close();
	}
	catch (exception& e)
	{
		writelog(string("Fehler: ")+e.what());
		return;
	}
	running = true;
	ready = false;
	mainthread = thread(&Streamserver::main,this);
}

void Streamserver::stop()
{
	if (ready)
		return;
	running = false;
	mainthread.join();
	ioc.restart();
	ready = true;
	writelog("Der Server wurde zurueckgesetzt.");
}

void Streamserver::main()
{
	try
	{
		while (running)
		{
			idle();
			if (!running)
				break;
			loop();
		}
	}
	catch (std::exception& err)
	{
		writelog(string("Unerwarteter Fehler:\n") + err.what());
	}
	while (sessions.size()>0)
		client_remove(sessions[0]);
	writelog("Der Streaming-Server wurde beendet");
}

void Streamserver::client_remove(Session* sess)
{
	if (sess->alive)
		sess->close();
	for (auto it = sessions.begin();it!=sessions.end();it++)
	{
		if (*it == sess)
		{
			sessions.erase(it);
			break;
		}
	}
	writelog(sess->ip.to_string()+" wurde getrennt.");
	delete sess;
}

void Streamserver::async_listen()
{
	if (!running)
		return;
	if (sessions.size()>=maxsessions)
		return;
	if (idle_session!=NULL)
		return;
	debug_log("Waiting for connection...");
	idle_session = new Session(ioc);
	acceptor.async_accept(idle_session->socket.next_layer(),boost::bind(&Streamserver::on_connection,this,_1));
}

void Streamserver::on_connection(const boost::system::error_code& err)
{
	debug_log("incoming connection");
	Session* sess = idle_session;
	idle_session = NULL;
	if (!running)
	{
		delete sess;
		return;
	}
	if (err)
	{
		debug_log(string("Connection error: ")+err.message());
		delete sess;
		return;
	}
	boost::asio::ip::address remote = sess->socket.next_layer().remote_endpoint().address();
	if (!is_allowed(remote))
	{
		writelog(remote.to_string()+" wurde abgewiesen.");
		delete sess;
		return;
	}
	writelog(remote.to_string()+" wurde verbunden.");
	sessions.push_back(sess);
	if (sessions.size() == maxsessions)
		writelog("Die maximale Verbindungsanzahl wurde erreicht!");
	sess->ip = remote;
	sess->start();
}

bool Streamserver::is_allowed(boost::asio::ip::address addr)
{
	bool allowed = false;
	listlock.lock();
	for (auto& ip: whitelist)
	{
		if (ip==addr)
		{
			allowed=true;
			break;
		}
	}
	listlock.unlock();
	return allowed;
}

void Streamserver::idle()
{
	writelog("Idle-Status wird eingenommen");
	while (sessions.size()<1 && running)
	{
		async_listen();
		this_thread::yield();
		ioc.poll();
	}
}

void Streamserver::loop()
{
	debug_log("loop()");
	async_listen();
	cam.open();
	cam.streamon();
	while (sessions.size()>0 && running)
	{
		async_listen();
		cam.read();
		ioc.poll();
		for (int i=sessions.size()-1;i>=0;i--)
		{
			if (!sessions[i]->alive || !is_allowed(sessions[i]->ip))
			{
				client_remove(sessions[i]);
				continue;
			}
			sessions[i]->transmit(cam.message_data);
		}
	}
	cam.close();
}

Streamserver::~Streamserver()
{
	stop();
}