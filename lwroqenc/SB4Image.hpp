#ifndef __SB4IMAGE_H__
#define __SB4IMAGE_H__

#include <string.h>
#include <vector>

#include "../common/lwmovie_coretypes.h"

class SB4ImagePlane
{
private:
	std::vector<lwmUInt8> m_data;
	lwmUInt32 m_stride;
	lwmUInt32 m_width, m_height;

public:
	inline void Init(lwmUInt32 width, lwmUInt32 height, lwmUInt8 def)
	{
		m_data.resize(width*height, def);
		m_stride = m_width = width;
		m_height = height;
	}

	inline const lwmUInt8 *Row(lwmUInt32 i) const
	{
		return &m_data[i*m_stride];
	}

	inline lwmUInt8 *Row(lwmUInt32 i)
	{
		return &m_data[i*m_stride];
	}

	inline lwmUInt32 Width() const
	{
		return m_width;
	}

	inline lwmUInt32 Height() const
	{
		return m_height;
	}

	inline void CopyTo(SB4ImagePlane *plane) const
	{
		memcpy(&plane->m_data[0], &m_data[0], m_data.size());
	}

	inline void HalveSizeInPlace()
	{
		lwmUInt32 newStride = m_stride / 2;
		lwmUInt32 w = m_width;
		lwmUInt32 h = m_height;
		lwmUInt32 stride = m_stride;
		lwmUInt8 *dataPtr = &m_data[0];

		for (lwmUInt32 y = 0; y<h / 2; y++)
			for (lwmUInt32 x = 0; x<w / 2; x++)
			{
				lwmFastUInt16 px0 = dataPtr[y * 2 * stride + x * 2];
				lwmFastUInt16 px1 = dataPtr[y * 2 * stride + x * 2 + 1];
				lwmFastUInt16 px2 = dataPtr[(y * 2 + 1)*stride + x * 2];
				lwmFastUInt16 px3 = dataPtr[(y * 2 + 1)*stride + x * 2 + 1];
				dataPtr[y*newStride + x] = static_cast<lwmUInt8>((px0 + px1 + px2 + px3 + 2) / 4);
			}

		m_data.resize(m_data.size() / 4);
		m_width /= 2;
		m_height /= 2;
		m_stride = newStride;
	}

	inline void DoubleSizeInPlace()
	{
		lwmUInt32 newStride = m_stride * 2;
		lwmUInt32 w = m_width;
		lwmUInt32 h = m_height;
		lwmUInt32 stride = m_stride;

		m_data.resize(m_data.size() * 4);

		lwmUInt8 *dataPtr = &m_data[0];

		for (lwmUInt32 iy = 0; iy<h; iy++)
		{
			lwmUInt32 y = (h - iy - 1);
			lwmUInt32 dy = y * 2;

			for (lwmUInt32 ix = 0; ix<w; ix++)
			{
				lwmUInt32 x = (w - ix - 1);
				lwmUInt32 dx = x * 2;

				dataPtr[dy*newStride + dx] =
					dataPtr[dy*newStride + dx + 1] =
					dataPtr[(dy + 1)*newStride + dx] =
					dataPtr[(dy + 1)*newStride + dx + 1] = dataPtr[y*stride + x];
			}
		}

		m_width *= 2;
		m_height *= 2;
		m_stride = newStride;
	}
};

class SB4Image
{
private:
	std::vector<SB4ImagePlane> m_planes;
	unsigned int m_width, m_height;

public:
	inline void Init(lwmUInt32 width, lwmUInt32 height)
	{
		m_planes.resize(3);
		m_planes[0].Init(width, height, 0);
		m_planes[1].Init(width, height, 128);
		m_planes[2].Init(width, height, 128);
		m_width = width;
		m_height = height;
	}

	inline const SB4ImagePlane *Plane(lwmUInt32 i) const
	{
		return &m_planes[i];
	}

	inline SB4ImagePlane *Plane(lwmUInt32 i)
	{
		return &m_planes[i];
	}

	inline lwmUInt32 Width() const
	{
		return m_width;
	}

	inline lwmUInt32 Height() const
	{
		return m_height;
	}

	inline void CopyTo(SB4Image *img) const
	{
		for (int i = 0; i < 3; i++)
			m_planes[i].CopyTo(&img->m_planes[i]);
	}
};

template <lwmUInt32 TD> class SB4Block
{
private:
	lwmUInt8 m_data[3][TD][TD];

public:
	inline SB4Block()
	{
	}

	inline SB4Block(const SB4Block<TD> &other)
	{
		memcpy(m_data, other.m_data, 3 * TD*TD);
	}

	inline void LoadFromImage(const SB4Image *img, lwmUInt32 imgx, lwmUInt32 imgy)
	{
		const SB4ImagePlane *plane;
		for (lwmUInt32 p = 0; p<3; p++)
		{
			plane = img->Plane(p);
			for (lwmUInt32 y = 0; y<TD; y++)
				memcpy(m_data[p][y], plane->Row(imgy + y) + imgx, TD);
		}
	}

	inline SB4Block(const SB4Image *img, lwmUInt32 imgx, lwmUInt32 imgy)
	{
		LoadFromImage(img, imgx, imgy);
	}

	inline const lwmUInt8 *Row(lwmUInt32 plane, lwmUInt32 i) const
	{
		return m_data[plane][i];
	}

	inline unsigned char *Row(lwmUInt32 plane, lwmUInt32 i)
	{
		return m_data[plane][i];
	}

	inline lwmUInt32 SquareDiff(const SB4Block<TD> &rs) const
	{
		lwmUInt32 dtotal = 0;
		for (int p = 0; p<3; p++)
			for (lwmUInt32 y = 0; y<TD; y++)
			{
				const lwmUInt8 *row1 = m_data[p][y];
				const lwmUInt8 *row2 = rs.m_data[p][y];
				for (lwmUInt32 x = 0; x<TD; x++)
				{
					lwmSInt32 d = *row1++ - *row2++;
					dtotal += static_cast<lwmUInt32>(d*d);
				}
			}

		return dtotal;
	}

	inline lwmUInt32 SquareDiffLimited(const SB4Block<TD> &rs, lwmUInt32 limit) const
	{
		lwmUInt32 dtotal = 0;
		for (int p = 0; p<3; p++)
			for (lwmUInt32 y = 0; y<TD; y++)
			{
				const lwmUInt8 *row1 = m_data[p][y];
				const lwmUInt8 *row2 = rs.m_data[p][y];
				for (lwmUInt32 x = 0; x<TD; x++)
				{
					lwmSInt32 d = *row1++ - *row2++;
					dtotal += static_cast<lwmUInt32>(d*d);
				}
				if (dtotal > limit)
					return dtotal;
			}

		return dtotal;
	}


	inline unsigned int IndexList(const SB4Block<TD> *v, lwmUInt32 count, lwmUInt32 seed, lwmUInt32 *diffp)
	{
		lwmUInt32 bestIndex = seed;
		lwmUInt32 bestDiff = this->SquareDiff(v[seed]);
		for (lwmUInt32 i = 0; i<count; i++)
		{
			if (i == seed)
				continue;

			lwmUInt32 diff = this->SquareDiffLimited(v[i], bestDiff);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				bestIndex = i;
			}
		}

		if (diffp)
			*diffp = bestDiff;

		return bestIndex;
	}

	inline lwmUInt32 IndexList(const SB4Block<TD> *v, lwmUInt32 count, lwmUInt32 seed)
	{
		return IndexList(v, count, seed, NULL);
	}

	inline void Blit(SB4Image *img, lwmUInt32 imgx, lwmUInt32 imgy) const
	{
		SB4ImagePlane *plane;
		for (lwmUInt32 p = 0; p<3; p++)
		{
			plane = img->Plane(p);
			for (lwmUInt32 y = 0; y<TD; y++)
				memcpy(plane->Row(imgy + y) + imgx, m_data[p][y], TD);
		}
	}

	inline lwmUInt32 LumaAverage() const
	{
		lwmUInt32 total = 0;
		for (lwmUInt32 i = 0; i<TD; i++)
			for (lwmUInt32 j = 0; j<TD; j++)
				total += m_data[0][i][j];
		total += (TD*TD) / 2;
		return total / (TD*TD);
	}
};

void SB4UnpackCB2(bool relative, const lwmSInt32 *cb2, SB4Block<2> *block);
SB4Block<8> SB4Enlarge4to8(const SB4Block<4> &b4);

#endif
