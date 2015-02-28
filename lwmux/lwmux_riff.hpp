#ifndef __LWMUX_RIFF_HPP__
#define __LWMUX_RIFF_HPP__

#include "../common/lwmovie_coretypes.h"
#include <string.h>
#include <vector>

class lwmOSFile;

namespace lwmovie
{
	namespace riff
	{
		struct SFourCC
		{
			lwmUInt8 fcc[4];

			inline bool operator == (const SFourCC &rs) const
			{
				return memcmp(fcc, rs.fcc, 4) == 0;
			}

			inline bool operator != (const SFourCC &rs) const
			{
				return memcmp(fcc, rs.fcc, 4) != 0;
			}

			inline SFourCC()
			{
			}

			inline SFourCC(const SFourCC &rs)
			{
				memcpy(fcc, rs.fcc, 4);
			}

			inline SFourCC(char c1, char c2, char c3, char c4)
			{
				fcc[0] = static_cast<lwmUInt8>(c1);
				fcc[1] = static_cast<lwmUInt8>(c2);
				fcc[2] = static_cast<lwmUInt8>(c3);
				fcc[3] = static_cast<lwmUInt8>(c4);
			}

			inline SFourCC(lwmUInt32 dword)
			{
				fcc[0] = static_cast<lwmUInt8>((dword >> 0) & 0xff);
				fcc[1] = static_cast<lwmUInt8>((dword >> 8) & 0xff);
				fcc[2] = static_cast<lwmUInt8>((dword >> 16) & 0xff);
				fcc[3] = static_cast<lwmUInt8>((dword >> 24) & 0xff);
			}
		};

		extern SFourCC ATOM_TYPE_RIFF;
		extern SFourCC ATOM_TYPE_LIST;

		class CRIFFAtom
		{
			SFourCC m_atomType;
			lwmUInt32 m_chunkSize;

		public:
			CRIFFAtom(const SFourCC &atomType, lwmUInt32 chunkSize);
			virtual ~CRIFFAtom();

			inline lwmUInt32 ChunkSize() const
			{
				return m_chunkSize;
			}

			inline const SFourCC &AtomType() const
			{
				return m_atomType;
			}
		};

		class CRIFFDataChunk : public CRIFFAtom
		{
			lwmUInt64 m_dataFileOffset;

		public:
			CRIFFDataChunk(const SFourCC &atomType, lwmUInt32 dataSize, lwmUInt64 dataFileOffset);
			inline lwmUInt64 FileOffset() const
			{
				return m_dataFileOffset;
			}
		};

		class CRIFFDataList : public CRIFFAtom
		{
			std::vector<CRIFFAtom*> m_children;
			SFourCC m_listFourCC;

		public:
			CRIFFDataList(const SFourCC &atomType, lwmUInt32 chunkSize, SFourCC listFourCC);
			virtual ~CRIFFDataList();
			void AddChild(CRIFFAtom *child);
			CRIFFDataChunk *FindDataChild(const SFourCC &atomType);
			CRIFFDataList *FindListChild(const SFourCC &atomType);
			std::vector<CRIFFDataList*> FindListChildren(const SFourCC &atomType);
			inline const std::vector<CRIFFAtom*> &Children() const
			{
				return m_children;
			}

			inline const SFourCC &ListFourCC() const
			{
				return m_listFourCC;
			}
		};

		CRIFFAtom *ParseAtom(lwmOSFile *inFile);
		lwmUInt16 ReadUInt16(lwmOSFile *inFile);
		lwmUInt32 ReadUInt32(lwmOSFile *inFile);
	}
}

#endif
