#include "lwmux_riff.hpp"
#include "lwmux_osfile.hpp"

lwmovie::riff::SFourCC lwmovie::riff::ATOM_TYPE_RIFF = lwmovie::riff::SFourCC('R', 'I', 'F', 'F');
lwmovie::riff::SFourCC lwmovie::riff::ATOM_TYPE_LIST = lwmovie::riff::SFourCC('L', 'I', 'S', 'T');

lwmovie::riff::CRIFFAtom::CRIFFAtom(const SFourCC &atomType, lwmUInt32 chunkSize)
	: m_atomType(atomType)
	, m_chunkSize(chunkSize)
{
}

lwmovie::riff::CRIFFAtom::~CRIFFAtom()
{
}

lwmovie::riff::CRIFFDataChunk::CRIFFDataChunk(const SFourCC &atomType, lwmUInt32 dataSize, lwmUInt64 dataFileOffset)
	: CRIFFAtom(atomType, dataSize)
	, m_dataFileOffset(dataFileOffset)
{
}

lwmovie::riff::CRIFFDataList::CRIFFDataList(const SFourCC &atomType, lwmUInt32 chunkSize, SFourCC listFourCC)
	: CRIFFAtom(atomType, chunkSize)
	, m_listFourCC(listFourCC)
{
}

lwmovie::riff::CRIFFDataList::~CRIFFDataList()
{
	for(std::vector<CRIFFAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
		delete const_cast<CRIFFAtom*>(*it);
}

void lwmovie::riff::CRIFFDataList::AddChild(CRIFFAtom *child)
{
	m_children.push_back(child);
}

lwmovie::riff::CRIFFDataChunk *lwmovie::riff::CRIFFDataList::FindDataChild(const SFourCC &atomType)
{
	for(std::vector<CRIFFAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
		if((*it)->AtomType() == atomType)
			return static_cast<CRIFFDataChunk*>(*it);
	return NULL;
}

lwmovie::riff::CRIFFDataList *lwmovie::riff::CRIFFDataList::FindListChild(const SFourCC &atomType)
{
	for(std::vector<CRIFFAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
	{
		bool isList = ((*it)->AtomType() == ATOM_TYPE_RIFF || (*it)->AtomType() == ATOM_TYPE_LIST);
		if(isList)
		{
			if(static_cast<CRIFFDataList*>(*it)->m_listFourCC == atomType)
				return static_cast<CRIFFDataList*>(*it);
		}
	}
	return NULL;
}

std::vector<lwmovie::riff::CRIFFDataList*> lwmovie::riff::CRIFFDataList::FindListChildren(const SFourCC &atomType)
{
	std::vector<CRIFFDataList*> v;
	for(std::vector<CRIFFAtom*>::iterator it=m_children.begin(), itEnd=m_children.end(); it != itEnd; ++it)
	{
		bool isList = ((*it)->AtomType() == ATOM_TYPE_RIFF || (*it)->AtomType() == ATOM_TYPE_LIST);
		if(isList)
		{
			if(static_cast<CRIFFDataList*>(*it)->m_listFourCC == atomType)
				v.push_back(static_cast<CRIFFDataList*>(*it));
		}
	}
	return v;
}


lwmUInt16 lwmovie::riff::ReadUInt16(lwmOSFile *inFile)
{
	lwmUInt8 buf[2];
	inFile->ReadBytes(buf, 2);
	return static_cast<lwmUInt16>(buf[0] | (buf[1] << 8));
}

lwmUInt32 lwmovie::riff::ReadUInt32(lwmOSFile *inFile)
{
	lwmUInt8 buf[4];
	inFile->ReadBytes(buf, 4);
	return static_cast<lwmUInt32>(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

static lwmovie::riff::CRIFFAtom *ParseChunk(lwmOSFile *inFile, lwmovie::riff::SFourCC atomType)
{
	lwmUInt32 chunkSize = lwmovie::riff::ReadUInt32(inFile);
	lwmUInt64 dataLoc = inFile->FilePos();
	inFile->Seek(static_cast<lwmSInt64>(chunkSize), lwmOSFile::SM_Current);
	return new lwmovie::riff::CRIFFDataChunk(atomType, chunkSize, dataLoc);
}

static lwmovie::riff::CRIFFAtom *ParseList(lwmOSFile *inFile, lwmovie::riff::SFourCC atomType)
{
	lwmUInt32 chunkSize = lwmovie::riff::ReadUInt32(inFile);
	lwmovie::riff::SFourCC listFourCC = lwmovie::riff::SFourCC(lwmovie::riff::ReadUInt32(inFile));

	lwmovie::riff::CRIFFDataList *dataList = new lwmovie::riff::CRIFFDataList(atomType, chunkSize, listFourCC);

	lwmUInt32 listDataSize = chunkSize - 4;
	while(listDataSize != 0)
	{
		lwmovie::riff::CRIFFAtom *subChunk = lwmovie::riff::ParseAtom(inFile);
		listDataSize -= subChunk->ChunkSize() + 8;
		dataList->AddChild(subChunk);
	}
	return dataList;
}

static lwmovie::riff::CRIFFAtom *ParseAtom_r(lwmOSFile *inFile)
{
	lwmovie::riff::SFourCC atomType = lwmovie::riff::SFourCC(lwmovie::riff::ReadUInt32(inFile));
	if(atomType == lwmovie::riff::ATOM_TYPE_RIFF || atomType == lwmovie::riff::ATOM_TYPE_LIST)
		return ParseList(inFile, atomType);
	return ParseChunk(inFile, atomType);
}

lwmovie::riff::CRIFFAtom *lwmovie::riff::ParseAtom(lwmOSFile *inFile)
{
	return ParseAtom_r(inFile);
}
