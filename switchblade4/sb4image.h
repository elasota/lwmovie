#ifndef __SB4IMAGE_H__
#define __SB4IMAGE_H__

#include <string.h>
#include <vector>
#include "sb4.h"


class SB4ImagePlane
{
private:
	std::vector<unsigned char> _data;
	unsigned int _stride;
	unsigned int _width, _height;

public:
	inline void Init(unsigned int width, unsigned int height, unsigned char def)
	{
		_data.resize(width*height, def);
		_stride = _width = width;
		_height = height;
	}

	inline const unsigned char *Row(unsigned int i) const
	{
		return &_data[i*_stride];
	}

	inline unsigned char *Row(unsigned int i)
	{
		return &_data[i*_stride];
	}

	inline unsigned int Width() const
	{
		return _width;
	}

	inline unsigned int Height() const
	{
		return _height;
	}

	inline void CopyTo(SB4ImagePlane *plane) const
	{
		memcpy(&plane->_data[0], &_data[0], _data.size());
	}

	inline void HalveSizeInPlace()
	{
		unsigned int newStride = _stride / 2;
		unsigned int w = _width;
		unsigned int h = _height;
		unsigned int stride = _stride;
		unsigned char *dataPtr = &_data[0];

		for(unsigned int y=0;y<h/2;y++)
			for(unsigned int x=0;x<w/2;x++)
			{
				dataPtr[y*newStride+x] = (unsigned char)(((int)dataPtr[y*2*stride+x*2] +
							 (int)dataPtr[y*2*stride+x*2+1] +
							 (int)dataPtr[(y*2+1)*stride+x*2] + 
							 (int)dataPtr[(y*2+1)*stride+x*2+1] + 2) / 4);
			}

		_data.resize(_data.size() / 4);
		_width /= 2;
		_height /= 2;
		_stride = newStride;
	}

	inline void DoubleSizeInPlace()
	{
		unsigned int newStride = _stride * 2;
		unsigned int w = _width;
		unsigned int h = _height;
		unsigned int stride = _stride;

		_data.resize(_data.size() * 4);

		unsigned char *dataPtr = &_data[0];

		for(unsigned int iy=0;iy<h;iy++)
		{
			unsigned int y = (h - iy - 1);
			unsigned int dy = y * 2;

			for(unsigned int ix=0;ix<w;ix++)
			{
				unsigned int x = (w - ix - 1);
				unsigned int dx = x * 2;

				dataPtr[dy*newStride+dx] = 
					dataPtr[dy*newStride+dx+1] = 
					dataPtr[(dy+1)*newStride+dx] = 
					dataPtr[(dy+1)*newStride+dx+1] = dataPtr[y*stride+x];
			}
		}

		_width *= 2;
		_height *= 2;
		_stride = newStride;
	}
};

class SB4Image
{
private:
	std::vector<SB4ImagePlane> _planes;
	unsigned int _width, _height;

public:
	inline void Init(unsigned int width, unsigned int height)
	{
		_planes.resize(3);
		_planes[0].Init(width, height, 0);
		_planes[1].Init(width, height, 128);
		_planes[2].Init(width, height, 128);
		_width = width;
		_height = height;
	}

	inline const SB4ImagePlane *Plane(unsigned int i) const
	{
		return &_planes[i];
	}

	inline SB4ImagePlane *Plane(unsigned int i)
	{
		return &_planes[i];
	}

	inline unsigned int Width() const
	{
		return _width;
	}

	inline unsigned int Height() const
	{
		return _height;
	}

	inline void CopyTo(SB4Image *img) const
	{
		int i;
		for(i=0;i<3;i++)
			_planes[i].CopyTo(&img->_planes[i]);
	}
};


template <unsigned int _D> class SB4Block
{
private:
	unsigned char _data[3][_D][_D];

public:

	inline SB4Block()
	{
	}

	inline SB4Block(const SB4Block<_D> &other)
	{
		memcpy(_data, other._data, 3*_D*_D);
	}

	inline void LoadFromImage(const SB4Image *img, unsigned int imgx, unsigned int imgy)
	{
		unsigned int p, y;
		const SB4ImagePlane *plane;
		for(p=0;p<3;p++)
		{
			plane = img->Plane(p);
			for(y=0;y<_D;y++)
				memcpy(_data[p][y], plane->Row(imgy+y) + imgx, _D);
		}
	}

	inline SB4Block(const SB4Image *img, unsigned int imgx, unsigned int imgy)
	{
		LoadFromImage(img, imgx, imgy);
	}

	inline const unsigned char *Row(unsigned int plane, unsigned int i) const
	{
		return _data[plane][i];
	}

	inline unsigned char *Row(unsigned int plane, unsigned int i)
	{
		return _data[plane][i];
	}

	inline unsigned int SquareDiff(const SB4Block<_D> &rs) const
	{
		unsigned int p, x, y, dtotal, d;
		const unsigned char *row1, *row2;

		dtotal = 0;
		for(p=0;p<3;p++)
			for(y=0;y<_D;y++)
			{
				row1 = _data[p][y];
				row2 = rs._data[p][y];
				for(x=0;x<_D;x++)
				{
					d = *row1++ - *row2++;
					dtotal += d*d;
				}
			}

		return dtotal;
	}

	inline unsigned int SquareDiffLimited(const SB4Block<_D> &rs, unsigned int limit) const
	{
		unsigned int p, x, y, dtotal, d;
		const unsigned char *row1, *row2;

		dtotal = 0;
		for(p=0;p<3;p++)
			for(y=0;y<_D;y++)
			{
				row1 = _data[p][y];
				row2 = rs._data[p][y];
				for(x=0;x<_D;x++)
				{
					d = *row1++ - *row2++;
					dtotal += d*d;
				}
				if(dtotal > limit)
					return dtotal;
			}

		return dtotal;
	}


	inline unsigned int SquareDiffDelta(const SB4Block<_D> *delta, const SB4Block<_D> &rs) const
	{
		unsigned int p, x, y, dtotal;
		int d;
		const unsigned char *row1, *row2, *rowDelta;

		dtotal = 0;
		for(p=0;p<3;p++)
			for(y=0;y<_D;y++)
			{
				rowDelta = delta->_data[p][y];
				row1 = _data[p][y];
				row2 = rs._data[p][y];
				for(x=0;x<_D;x++)
				{
					d = (*rowDelta++) + ((int)(*row2++) * 2 - 255);
					if(d > 255) d = 255; else if(d < 0) d = 0;
					d -= *row1++;
					dtotal += d*d;
				}
			}

		return dtotal;
	}

	inline unsigned int SquareDiffLimitedDelta(const SB4Block<_D> *delta, const SB4Block<_D> &rs, unsigned int limit) const
	{
		unsigned int p, x, y, dtotal;
		int d;
		const unsigned char *row1, *row2, *rowDelta;

		dtotal = 0;
		for(p=0;p<3;p++)
			for(y=0;y<_D;y++)
			{
				rowDelta = delta->_data[p][y];
				row1 = _data[p][y];
				row2 = rs._data[p][y];
				for(x=0;x<_D;x++)
				{
					d = (*rowDelta++) + ((int)(*row2++) * 2 - 255);
					if(d > 255) d = 255; else if(d < 0) d = 0;
					d -= *row1++;
					dtotal += d*d;
				}
				if(dtotal > limit)
					return dtotal;
			}

		return dtotal;
	}

	inline unsigned int IndexList(const SB4Block<_D> *delta, const SB4Block<_D> *v, unsigned int count, unsigned int seed, unsigned int *diffp)
	{
		unsigned int i, bestDiff, diff, bestIndex;

		bestIndex = seed;
		if(delta)
			bestDiff = this->SquareDiffDelta(delta, v[seed]);
		else
			bestDiff = this->SquareDiff(v[seed]);
		for(i=0;i<count;i++)
		{
			if(i == seed)
				continue;

			if(delta)
				diff = this->SquareDiffLimitedDelta(delta, v[i], bestDiff);
			else
				diff = this->SquareDiffLimited(v[i], bestDiff);
			if(diff < bestDiff)
			{
				bestDiff = diff;
				bestIndex = i;
			}
		}

		if(diffp)
			*diffp = bestDiff;

		return bestIndex;
	}

	inline unsigned int IndexList(const SB4Block<_D> *v, unsigned int count, unsigned int seed)
	{
		return IndexList(v, count, seed, NULL);
	}

	inline unsigned int IndexList(const SB4Block<_D> *delta, const SB4Block<_D> *v, unsigned int count, unsigned int seed)
	{
		return IndexList(delta, v, count, seed, NULL);
	}

	inline void Blit(SB4Image *img, unsigned int imgx, unsigned int imgy) const
	{
		unsigned int p, y;
		SB4ImagePlane *plane;
		for(p=0;p<3;p++)
		{
			plane = img->Plane(p);
			for(y=0;y<_D;y++)
				memcpy(plane->Row(imgy+y) + imgx, _data[p][y], _D);
		}
	}


	inline void DBlit(const SB4Block<_D> *delta, SB4Image *img, unsigned int imgx, unsigned int imgy) const
	{
		unsigned int p, y, x;
		int d;
		SB4ImagePlane *plane;
		const unsigned char *cbRow, *inRow;
		unsigned char *outRow;

		for(p=0;p<3;p++)
		{
			plane = img->Plane(p);
			for(y=0;y<_D;y++)
			{
				outRow = plane->Row(imgy+y) + imgx;
				inRow = delta->_data[p][y];
				cbRow = _data[p][y];

				for(x=0;x<_D;x++)
				{
					d = *inRow++;

					d += ((int)(*cbRow++) * 2 - 255);
					if(d < 0)
						d = 0;
					else if(d > 255)
						d = 255;
					*outRow++ = d;
				}
			}
		}
	}

	inline unsigned int LumaAverage() const
	{
		int i,j;
		unsigned int total=0;
		for(i=0;i<_D;i++)
			for(j=0;j<_D;j++)
				total += _data[0][i][j];
		total += (_D*_D)/2;
		return total / (_D*_D);
	}
};

void UnpackCB2(bool relative, const int *cb2, SB4Block<2> *block);
void UnpackAllCB2(const unsigned char cb2[1536], SB4Block<2> *cb2unpacked);
void UnpackAllCB4(const SB4Block<2> cb2unpacked[256], const unsigned char cb4[1024], SB4Block<4> *cb4unpacked);
SB4Block<8> Enlarge4to8(const SB4Block<4> &b4);

#endif
