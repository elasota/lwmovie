#ifndef __LWMUX_ESCAPE_HPP__
#define __LWMUX_ESCAPE_HPP__

#include "../common/lwmovie_coretypes.h"

lwmUInt32 lwmComputeEscapes(const lwmUInt8 *bytes, lwmUInt32 unescapedSize);
void lwmGenerateEscapedBytes(lwmUInt8 *outBytes, const lwmUInt8 *inBytes, lwmUInt32 unescapedSize);

#endif
