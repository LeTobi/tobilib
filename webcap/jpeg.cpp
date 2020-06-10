#include "jpeg.h"
#include <jpeglib.h>
#include "../general/exception.hpp"

namespace tobilib
{
	void jpeg_encoder::encode_yuv (const basicBuffer& rawdata, int width, int height)
	{
		// Hier fehlt eine Exception-Implementierung f�r libjpeg
		// BETE DAF�R, DASS KEIN FEHLER PASSIERT
		jpeg_compress_struct compression;
		jpeg_error_mgr error;
		compression.err = jpeg_std_error(&error);
		jpeg_create_compress(&compression);
		try {
			buffer.open(rawdata.used);
		} catch (Exception& e) {
			e.trace.push_back("jpeg_encoder::encode_yuv()");
			throw e;
		}
		jpeg_stdio_dest(&compression,buffer.file());
		compression.image_width = width;
		compression.image_height = height;
		compression.input_components = 3;
		compression.in_color_space = JCS_YCbCr;
		jpeg_set_defaults(&compression);
		jpeg_start_compress(&compression,true);
		for (int i=0;i<rawdata.used;i+=3*width)
		{
			// ACHTUNG: Hier wird const weggecastet, weil libjpeg zu bl�d f�r const qualifiers ist.
			unsigned char * offset = reinterpret_cast<unsigned char*>(const_cast<char*>(rawdata.start()+i));
			jpeg_write_scanlines(&compression,&offset,1);
		}
		jpeg_finish_compress(&compression);
		buffer.close();
		jpeg_destroy_compress(&compression);
	}
}