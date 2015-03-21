#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "lwmux_osfile.hpp"

const int INVALID_FILE = -1;

class lwmOSFileImpl
{
private:
	FILE *m_file;

public:
	lwmOSFileImpl()
	{
		m_file = NULL;
	}

	~lwmOSFileImpl()
	{
		if(m_file)
		{
			fclose(m_file);
		}
		m_file = NULL;
	}

	bool Open(const char *path, lwmOSFile::EFileMode fileMode)
	{
		FILE *file;
		const char *flags = NULL;
		switch(fileMode)
		{
		case lwmOSFile::FM_Create:
			flags = "wb";
			break;
		case lwmOSFile::FM_Read:
			flags = "r";
			break;
		case lwmOSFile::FM_ReadWrite:
			flags = "w+b";
			break;
		case lwmOSFile::FM_Append:
			flags = "ab";
			break;
		default:
			return false;
		};
		file = fopen(path, flags);
		if(!file)
			return false;
		m_file = file;
		return true;
	}

	lwmUInt64 FilePos() const
	{
		return ftello(m_file);
	}

	void Seek(lwmSInt64 pos, lwmOSFile::ESeekMode seekMode)
	{
		int moveMethod = SEEK_SET;
		switch(seekMode)
		{
		case lwmOSFile::SM_Start:
			moveMethod = SEEK_SET;
			break;
		case lwmOSFile::SM_Current:
			moveMethod = SEEK_CUR;
			break;
		case lwmOSFile::SM_End:
			moveMethod = SEEK_END;
			break;
		};
		fseeko(m_file, pos, moveMethod);
	}
	
	void Seek(lwmUInt64 pos)
	{
		fseeko(m_file, pos, SEEK_SET);
	}

	lwmUInt64 ReadBytes(void *outBuffer, lwmUInt64 byteCount)
	{
		return fread(outBuffer, 1, static_cast<size_t>(byteCount), m_file);
	}
	
	lwmUInt64 WriteBytes(const void *inBuffer, lwmUInt64 byteCount)
	{
		return fwrite(inBuffer, 1, static_cast<size_t>(byteCount), m_file);
	}

};

lwmOSFile::lwmOSFile()
{
	impl = NULL;
	impl = new lwmOSFileImpl();
}

lwmOSFile::~lwmOSFile()
{
	if(impl)
		delete impl;
}

lwmUInt64 lwmOSFile::FilePos() const
{
	return impl->FilePos();
}

void lwmOSFile::Seek(lwmUInt64 pos)
{
	impl->Seek(pos);
}

void lwmOSFile::Seek(lwmSInt64 pos, lwmOSFile::ESeekMode seekMode)
{
	impl->Seek(pos, seekMode);
}

lwmOSFile *lwmOSFile::Open(const char *path, EFileMode fileMode)
{
	if(!strcmp(path, "-"))
		return NULL;
	lwmOSFileImpl *impl = new lwmOSFileImpl();
	if(!impl->Open(path, fileMode))
	{
		delete impl;
		return NULL;
	}

	lwmOSFile *osFile = new lwmOSFile();
	osFile->impl = impl;

	return osFile;
}

lwmUInt64 lwmOSFile::ReadBytes(void *outBuffer, lwmUInt64 byteCount)
{
	return impl->ReadBytes(outBuffer, byteCount);
}

lwmUInt64 lwmOSFile::WriteBytes(const void *inBuffer, lwmUInt64 byteCount)
{
	return impl->WriteBytes(inBuffer, byteCount);
}
