#ifndef TC_JPEG_H
#define TC_JPEG_H

#include "buffer.h"

namespace tobilib
{
	class jpeg_encoder
	{
	private:
		BufferFile buffer;
		
	public:
		const basicBuffer& data() const {return buffer;};
		void encode_yuv(const basicBuffer&, int, int);
	};
}

#ifdef TC_AS_HPP
	#include "jpeg.cpp"
#endif

#endif