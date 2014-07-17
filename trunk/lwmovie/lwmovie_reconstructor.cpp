#include "lwmovie_videotypes.hpp"
#include <string.h>

void j_rev_dct_sse2( short data[64] );

bool lwmovie::lwmReconstructor::Initialize(const lwmSAllocator *alloc, lwmUInt32 width, lwmUInt32 height)
{
	m_alloc = *alloc;

	lwmLargeUInt numMBlocks = ((width + 15) / 16) * ((height + 15) / 16);
	m_mblocks = static_cast<lwmMBlockInfo*>(m_alloc.allocFunc(m_alloc.opaque, numMBlocks * sizeof(lwmMBlockInfo)));
	m_dctBlocks = static_cast<lwmDCTBLOCK*>(m_alloc.allocFunc(m_alloc.opaque, numMBlocks * 6 * sizeof(lwmDCTBLOCK)));
	m_blocks = static_cast<lwmBlockInfo*>(m_alloc.allocFunc(m_alloc.opaque, numMBlocks * 6 * sizeof(lwmBlockInfo)));

	return true;	// TODO: Failure
}

void lwmovie::lwmReconstructor::SetBlockInfo(lwmSInt32 sbAddress, bool zero_block_flag)
{
	m_blocks[sbAddress].SetReconInfo(zero_block_flag);
}

void lwmovie::lwmReconstructor::SetMBlockInfo(lwmSInt32 mbAddress, lwmSInt32 recon_right_for, lwmSInt32 recon_down_for, lwmSInt32 recon_right_back, lwmSInt32 recon_down_back)
{
	m_mblocks[mbAddress].SetReconInfo(recon_right_for, recon_down_for, recon_right_back, recon_down_back);
}

void lwmovie::lwmReconstructor::PutMB(lwmSInt32 address, const lwmMBlockInfo *block)
{
	this->m_mblocks[address] = *block;
}

lwmovie::lwmDCTBLOCK *lwmovie::lwmReconstructor::StartReconBlock(lwmSInt32 address)
{
	lwmSInt32 mbAddress = address / 6;
	m_activeMBlock = m_mblocks + mbAddress;
	m_activeBlock = m_blocks + address;
	m_activeDCTBlock = m_dctBlocks + address;
	return m_activeDCTBlock;
}

void lwmovie::lwmReconstructor::CommitZero()
{
	memset(m_activeDCTBlock->data, 0, sizeof(m_activeDCTBlock->data));
}

void lwmovie::lwmReconstructor::CommitSparse(lwmUInt8 lastCoeffPos, lwmSInt16 lastCoeff)
{
	memset(m_activeDCTBlock->data, 0, sizeof(m_activeDCTBlock->data));
	m_activeDCTBlock->data[lastCoeffPos] = lastCoeff;
	CommitFull();
}

void lwmovie::lwmReconstructor::CommitFull()
{
	j_rev_dct_sse2(m_activeDCTBlock->data);
}
