#include "lwmux_escape.hpp"

lwmUInt32 lwmComputeEscapes(const lwmUInt8 *bytes, lwmUInt32 unescapedSize)
{
	lwmUInt32 escapedSize = unescapedSize;
	for(lwmUInt32 i=2;i<unescapedSize;i++)
	{
		if(bytes[i] == 1 && bytes[i-1] == 0 && bytes[i-2] == 0)
			escapedSize++;
	}
	return escapedSize;
}

void lwmGenerateEscapedBytes(lwmUInt8 *outBytes, const lwmUInt8 *inBytes, lwmUInt32 unescapedSize)
{
	outBytes[0] = inBytes[0];
	outBytes[1] = inBytes[1];
	outBytes += 2;
	for(lwmUInt32 i=2;i<unescapedSize;i++)
	{
		*outBytes++ = inBytes[i];
		if(inBytes[i] == 1 && inBytes[i-1] == 0 && inBytes[i-2] == 0)
			*outBytes++ = 0xfe;
	}
}