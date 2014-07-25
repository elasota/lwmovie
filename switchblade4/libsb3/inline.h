/*
Copyright (C)2004 Eric Lasota/Orbiter Productions

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __RC3_INLINE_H__
#define __RC3_INLINE_H__

#ifndef __cplusplus
#  ifdef __MSC_VER
#    define rc_inline __inline
#  else
#    ifdef __GNUC__
#      define rc_inline static __inline__
#    else
#      define rc_inline static
#    endif
#  endif
#else
#  warning using C++ inlines
#  define rc_inline inline
#endif

#endif
