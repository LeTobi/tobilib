#ifndef TC_CAPTURE_H
#define TC_CAPTURE_H

#include "../stringplus/stringplus.h"
#include "buffer.h"
#include "jpeg.h"

#include <linux/videodev2.h>

namespace tobilib
{
	class Capture
	{
	public:
		enum class State {
			closed,
			open,
			streaming
		};
		
		const State& status() const {return _status;};
		
		//Schritt1
		void open(const StringPlus&);
		
		//Schritt2
		StringPlus check();
		void streamon();
		void read();
		void streamoff();
		
		//Schritt3
		void close();
		
		//nach read()
		const basicBuffer& data() const {return encoder.data();};
		
	private:
		int device = -1;
		StringPlus devname;
		int imgwidth;
		int imgheight;
		State _status = State::closed;
		v4l2_buffer bufferinfo;
		
		MappedBuffer cambuffer;
		Buffer rawdata;
		jpeg_encoder encoder;
		
		bool check_format(const v4l2_format&);
	};
}

#ifdef TC_AS_HPP
	#include "capture.cpp"
#endif

#endif