#ifndef TC_BUFFER_H
#define TC_BUFFER_H
#include "../stringplus/stringplus.h"
#include <cstdio>

namespace tobilib
{
	class basicBuffer
	{
	protected:
		char* _start = NULL;
		int _length = 0;
		
	public:
		basicBuffer(const basicBuffer&) = delete;
		basicBuffer& operator=(const basicBuffer&) = delete;
		basicBuffer() {};
	
		StringPlus toString() const;
		char* start() {return _start;};
		const char* start() const { return _start; };
		int length() const {return _length;};
		int used = 0;
		virtual void clear() {};
	};
	
	class Buffer: public virtual basicBuffer
	{
	public:
		void create(int);
		void clear();
		~Buffer();
	};
	
	class MappedBuffer: public basicBuffer
	{
	public:
		MappedBuffer() {};
		void map(int, int, int);
		void clear();
		
		~MappedBuffer();
	};
	
	class BufferFile: public virtual basicBuffer, private Buffer
	{
	private:
		FILE* _file = NULL;
		
	public:
		void open(int);
		void close();
		FILE* file() {return _file;};
		
		~BufferFile();
	};
}

#ifdef TC_AS_HPP
	#include "buffer.cpp"
#endif

#endif