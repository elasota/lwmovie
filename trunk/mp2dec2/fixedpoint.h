/*
Copyright (c) 2012 Eric Lasota

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __FIXEDPOINT_H__
#define __FIXEDPOINT_H__

#include "config.h"

#ifndef MP2DEC_FLOATINGPOINT

struct MP2Dec_Fixed32Base
{
        int _i;
};

#ifdef __cplusplus

template<int _FracBits>
class MP2Dec_Fixed32 : public MP2Dec_Fixed32Base
{
private:
        int SLS(int v, int bits);
        int SRS(int v, int bits);

public:
        int RawData() const;

        MP2Dec_Fixed32();

        template<int _RSFracBits>
        MP2Dec_Fixed32(const MP2Dec_Fixed32<_RSFracBits> &rs);

        MP2Dec_Fixed32(const float &f);
        MP2Dec_Fixed32(const unsigned int i);
        MP2Dec_Fixed32(const double &f);

        MP2Dec_Fixed32<_FracBits> & operator +=(const MP2Dec_Fixed32<_FracBits> &rs);

        template<int _RSFracBits>
        MP2Dec_Fixed32<_FracBits> & operator *=(const MP2Dec_Fixed32<_RSFracBits> &rs);
        MP2Dec_Fixed32<_FracBits> & operator -=(const MP2Dec_Fixed32<_FracBits> &rs);

        MP2Dec_Fixed32<_FracBits> operator +(const MP2Dec_Fixed32<_FracBits> &rs) const;

        MP2Dec_Fixed32<_FracBits> operator -(const MP2Dec_Fixed32<_FracBits> &rs) const;

        MP2Dec_Fixed32<_FracBits> operator -() const;


        template<int _RSFracBits>
        MP2Dec_Fixed32<_FracBits> operator *(const MP2Dec_Fixed32<_RSFracBits> &rs) const;

        int MulAndRound(int rs) const;
        int Round() const;
};


#endif  // __cplusplus

#endif  // !MP2DEC_FLOATINGPOINT

#endif

