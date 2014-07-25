#ifndef __SWITCHBLADE_3_INTERNAL_H__
#define __SWITCHBLADE_3_INTERNAL_H__

#include "sb3.h"

#include "inline.h"

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long i32;
typedef short i16;
typedef char i8;

typedef u32 uint;
//typedef unsigned int uint;

typedef struct
{
	u8 y[4];
	u8 u,v;

	u8 pad1, pad2;
} sb3_yuvcluster2_t;

typedef struct
{
	sb3_yuvcluster2_t block[4];
} sb3_yuvcluster4_t;



#define SB3_PERTURBATION_BASE_POWER      6



#endif
