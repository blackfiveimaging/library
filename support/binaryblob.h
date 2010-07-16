#ifndef BINARYBLOB_H
#include <iostream>

#include <cstdio>
#include <cstdlib>

#include "util.h"

// Class to handle the loading of a binary blob from a file, taking care of such tedious details as
// determining the filesize, and translating the filename from UTF8 to wchar_t if on Windows.

class BinaryBlob
{
	public:
	BinaryBlob() : pointer(NULL), size(0), owned(false)
	{
	}
	BinaryBlob(const char *filename) : pointer(NULL), size(0), owned(false)
	{
		Load(filename);
	}
	virtual ~BinaryBlob()
	{
		if(owned && pointer)
			delete[] pointer;
	}
	virtual unsigned char *Load(const char *filename)
	{
		if(owned && pointer)
			delete[] pointer;
		pointer=NULL;

		FILE *f;
		if(!(f = FOpenUTF8(filename, "rb")))
			throw "Can't open file";
		
		fseek(f,0,SEEK_END);
		size=ftell(f);
		fseek(f,0,SEEK_SET);

		Debug[TRACE] << "Loading binary blob " << filename << " of size: " << size << std::endl;

		pointer=new unsigned char[size];
		owned=true;
		size_t readlen = fread(pointer, 1, size, f);
		fclose(f);
		if(readlen!=size)
			throw "Binary blob reading failed";
		return(pointer);
	}
	int GetSize()
	{
		return(size);
	}
	unsigned char &operator[](int idx)
	{
		return(pointer[idx]);
	}
	protected:
	unsigned char *pointer;
	size_t size;
	bool owned;
};

#endif

