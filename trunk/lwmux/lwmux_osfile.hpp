#ifndef __LWMUX_OSFILE_HPP__
#define __LWMUX_OSFILE_HPP__

#include "../lwmovie/lwmovie_types.hpp"

class lwmOSFileImpl;

class lwmOSFile
{
public:
	enum EFileMode
	{
		FM_Create,
		FM_Read,
		FM_ReadWrite,
		FM_Append,
	};

	enum ESeekMode
	{
		SM_Start,
		SM_Current,
		SM_End,
	};

	lwmOSFile();
	virtual ~lwmOSFile();

	lwmUInt64 FilePos() const;
	void Seek(lwmUInt64 pos);
	void Seek(lwmSInt64 pos, ESeekMode seekMode);
	lwmUInt64 ReadBytes(void *outBuffer, lwmUInt64 byteCount);
	lwmUInt64 WriteBytes(const void *inBuffer, lwmUInt64 byteCount);
	static lwmOSFile *Open(const char *path, EFileMode fileMode);

private:
	lwmOSFileImpl *impl;
};

#endif
