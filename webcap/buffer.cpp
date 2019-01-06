#include "buffer.h"
#include "../stringplus/stringplus.h"
#include "../general/exception.hpp"
#include <sys/mman.h> // mmap

namespace tobilib
{
	StringPlus basicBuffer::toString() const
	{
		StringPlus out (0,used);
		for (int i=0;i<used;i++)
		{
			out[i] += _start[i];
		}
		return out;
	}
	
	void Buffer::create(int size)
	{
		clear();
		_length = size;
		_start = new char[size];
		used = 0;
	}
	
	void Buffer::clear()
	{
		if (_length>0)
			delete _start;
		used = 0;
		_length = 0;
		_start = NULL;
	}
	
	Buffer::~Buffer()
	{
		clear();
	}
	
	void MappedBuffer::map(int device, int offset, int len)
	{
		clear();
		_start = static_cast<char*>(mmap(NULL,len,PROT_READ | PROT_WRITE, MAP_SHARED, device, offset));
		if (_start == NULL)
		{
			Exception err ("Fehler beim Mapping des Device-Buffers");
			err.trace.push_back("MappedBuffer::map()");
			throw err;
		}
		_length = len;
		used = 0;
	}
	
	void MappedBuffer::clear()
	{
		munmap(_start,_length);
		_start = NULL;
		_length = 0;
		used = 0;
	}
	
	MappedBuffer::~MappedBuffer()
	{
		clear();
	}
	
	void BufferFile::open(int size)
	{
		close();
		if (_length<size)
		{
			clear();
			create(size);
		}
		_file = fmemopen(start(),size,"w");
		if (_file == NULL)
		{
			Exception err ("Fehler beim erstellen der Speicherdatei");
			err.trace.push_back("BufferFile::open()");
			throw err;
		}
	}
	
	void BufferFile::close()
	{
		if (_file == NULL)
			return;
		used = ftell(_file);
		fclose(_file);
		_file = NULL;
	}
	
	BufferFile::~BufferFile()
	{
		close();
	}
}