/*
* Copyright (c) 2015 Eric Lasota
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#ifndef __LWMUX_OSFILE_HPP__
#define __LWMUX_OSFILE_HPP__

#include <stdio.h>

#include "../common/lwmovie_coretypes.h"

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
	void Attach(FILE *f);

private:
	lwmOSFileImpl *impl;
};

#endif
