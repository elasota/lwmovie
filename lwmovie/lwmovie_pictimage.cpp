#include <stddef.h>

#include "lwmovie_videotypes.hpp"
#include "lwmovie_external_types.h"

lwmovie::lwmPictImage::lwmPictImage(const lwmSAllocator *palloc, lwmLargeUInt w, lwmLargeUInt h)
{
	this->alloc = *palloc;

	/* Create a YV12 image (Y + V + U) */
	this->luminance = static_cast<lwmUInt8*>(palloc->allocFunc(palloc->opaque, w*h/4));
	this->Cr = static_cast<lwmUInt8*>(palloc->allocFunc(palloc->opaque, w*h/4));
	this->Cb = static_cast<lwmUInt8*>(palloc->allocFunc(palloc->opaque, w*h/4));

	this->createdOk = (luminance != NULL && Cr != NULL && Cb != NULL);
	this->m_isReserved = false;
}

lwmovie::lwmPictImage::~lwmPictImage()
{
	if(this->luminance)
		alloc.freeFunc(alloc.opaque, this->luminance);
	if(this->Cb)
		alloc.freeFunc(alloc.opaque, this->Cb);
	if(this->Cr)
		alloc.freeFunc(alloc.opaque, this->Cr);
}

lwmUInt8 *lwmovie::lwmPictImage::GetLuminancePlane() const
{
	return this->luminance;
}

lwmUInt8 *lwmovie::lwmPictImage::GetCrPlane() const
{
	return this->Cr;
}

lwmUInt8 *lwmovie::lwmPictImage::GetCbPlane() const
{
	return this->Cb;
}

bool lwmovie::lwmPictImage::IsCreated() const
{
	return this->createdOk;
}

bool lwmovie::lwmPictImage::IsReserved() const
{
	return this->m_isReserved;
}

void lwmovie::lwmPictImage::Reserve()
{
	this->m_isReserved = true;
}

void lwmovie::lwmPictImage::Unreserve()
{
	this->m_isReserved = false;
}

lwmovie::lwmPictImage::lwmPictImage()
{
	this->luminance = NULL;
	this->Cr = NULL;
	this->Cb = NULL;

	this->createdOk = (luminance != NULL && Cr != NULL && Cb != NULL);
	this->m_isReserved = false;
}