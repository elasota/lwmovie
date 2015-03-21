#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "lwmux_osfile.hpp"

class lwmOSFileImpl
{
private:
	HANDLE m_hfile;
	lwmUInt8 m_readCache[1000];
	lwmLargeUInt m_cacheAvailable;
	const lwmUInt8 *m_cachePtr;

public:
	lwmOSFileImpl()
	{
		m_hfile = INVALID_HANDLE_VALUE;
		m_cachePtr = m_readCache;
		m_cacheAvailable = 0;
	}

	~lwmOSFileImpl()
	{
		if(m_hfile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hfile);
		}
		m_hfile = INVALID_HANDLE_VALUE;
	}

	bool Open(const char *path, lwmOSFile::EFileMode fileMode)
	{
		HANDLE file;
		UINT uStyle;
		DWORD access;
		DWORD shareMode;
		DWORD createDisposition;
		switch(fileMode)
		{
		case lwmOSFile::FM_Create:
			uStyle = OF_CREATE;
			access = GENERIC_READ | GENERIC_WRITE;
			shareMode = 0;
			createDisposition = CREATE_ALWAYS;
			break;
		case lwmOSFile::FM_Read:
			uStyle = OF_READ;
			access = GENERIC_READ;
			shareMode = FILE_SHARE_READ;
			createDisposition = OPEN_EXISTING;
			break;
		case lwmOSFile::FM_ReadWrite:
			uStyle = OF_CREATE | OF_READWRITE;
			access = GENERIC_READ | GENERIC_WRITE;
			shareMode = 0;
			createDisposition = OPEN_ALWAYS;
			break;
		case lwmOSFile::FM_Append:
			uStyle = OF_READWRITE;
			access = GENERIC_READ | GENERIC_WRITE;
			shareMode = 0;
			createDisposition = OPEN_ALWAYS;
			break;
		default:
			return false;
		};
		file = CreateFile(path, access, shareMode, NULL, createDisposition, 0, NULL);
		if(file == INVALID_HANDLE_VALUE)
			return false;
		m_hfile = file;
		return true;
	}

	lwmUInt64 FilePos() const
	{
		LARGE_INTEGER pos;
		LARGE_INTEGER offs;
		offs.QuadPart = 0LL;
		SetFilePointerEx(m_hfile, offs, &pos, FILE_CURRENT);
		return static_cast<lwmUInt64>(pos.QuadPart) - m_cacheAvailable;
	}

	void Seek(lwmSInt64 pos, lwmOSFile::ESeekMode seekMode)
	{
		DWORD moveMethod = FILE_BEGIN;
		switch(seekMode)
		{
		case lwmOSFile::SM_Start:
			moveMethod = FILE_BEGIN;
			break;
		case lwmOSFile::SM_Current:
			moveMethod = FILE_CURRENT;
			pos -= static_cast<lwmSInt64>(m_cacheAvailable);
			break;
		case lwmOSFile::SM_End:
			moveMethod = FILE_END;
			break;
		};
		LARGE_INTEGER dist;
		dist.QuadPart = static_cast<LONGLONG>(pos);
		SetFilePointerEx(m_hfile, dist, NULL, moveMethod);

		if(seekMode != lwmOSFile::SM_Current || pos != 0)
		{
			m_cachePtr = m_readCache;
			m_cacheAvailable = 0;
		}
	}
	
	void Seek(lwmUInt64 pos)
	{
		LARGE_INTEGER dist;
		dist.QuadPart = static_cast<LONGLONG>(pos);
		SetFilePointerEx(m_hfile, dist, NULL, FILE_BEGIN);
		m_cachePtr = m_readCache;
		m_cacheAvailable = 0;
	}

	lwmUInt64 ReadBytes(void *outBuffer, lwmUInt64 byteCount)
	{
		if(byteCount == 1 && m_cacheAvailable)
		{
			*static_cast<lwmUInt8*>(outBuffer) = *m_cachePtr++;
			m_cacheAvailable--;
			return 1;
		}

		lwmUInt64 total = 0;
		while(byteCount)
		{
			if(m_cacheAvailable == 0)
			{
				DWORD numCacheRead;
				BOOL readAny = ReadFile(m_hfile, m_readCache, sizeof(m_readCache), &numCacheRead, NULL);
				if(readAny)
				{
					m_cachePtr = m_readCache;
					m_cacheAvailable = static_cast<lwmLargeUInt>(numCacheRead);
				}

				if(!numCacheRead)
					break;
			}
			
			lwmLargeUInt chunkRead = (byteCount > m_cacheAvailable) ? m_cacheAvailable : static_cast<lwmLargeUInt>(byteCount);
			memcpy(outBuffer, m_cachePtr, chunkRead);
			m_cachePtr += chunkRead;
			m_cacheAvailable -= chunkRead;
			outBuffer = static_cast<lwmUInt8*>(outBuffer) + chunkRead;
			byteCount -= chunkRead;
			total += chunkRead;
		}
		return total;
	}
	
	lwmUInt64 WriteBytes(const void *inBuffer, lwmUInt64 byteCount)
	{
		if(m_cacheAvailable)
		{
			LARGE_INTEGER backDist;
			backDist.QuadPart = -static_cast<lwmSInt64>(m_cacheAvailable);
			SetFilePointerEx(m_hfile, backDist, NULL, FILE_CURRENT);
			m_cachePtr = m_readCache;
			m_cacheAvailable = 0;
		}

		lwmUInt64 total = 0;
		while(byteCount)
		{
			DWORD chunkWrite = (byteCount > 0x7fffffffLL) ? 0x7fffffff : static_cast<DWORD>(byteCount);
			DWORD numWrite;
			BOOL writeAny = WriteFile(m_hfile, inBuffer, chunkWrite, &numWrite, NULL);
			if(!writeAny)
				chunkWrite = 0;
			total += static_cast<lwmUInt32>(numWrite);
			byteCount -= static_cast<lwmUInt32>(numWrite);
			inBuffer = static_cast<const lwmUInt8*>(inBuffer) + numWrite;

			if(numWrite != chunkWrite)
				break;
		}
		return total;
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
