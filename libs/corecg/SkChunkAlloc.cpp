#include "SkChunkAlloc.h"

SkChunkAlloc::~SkChunkAlloc()
{
	this->reset();
}

void SkChunkAlloc::reset()
{
	Block* block = fBlock;

	while (block)
	{
		Block* next = block->fNext;
		sk_free(block);
		block = next;
	}
	fBlock = nil;
}

void* SkChunkAlloc::alloc(size_t bytes, AllocFailType ftype)
{
	bytes = SkAlign4(bytes);

	if (fBlock == nil || bytes > fBlock->fFreeSize)
	{
		size_t	size = SkMax32((S32)bytes, (S32)fMinSize);
		Block*	block = (Block*)sk_malloc_flags(sizeof(Block) + size, ftype == kThrow_AllocFailType ? SK_MALLOC_THROW : 0);
		if (block == nil)
			return nil;

		block->fNext = fBlock;
		block->fFreeSize = size;
		block->fFreePtr = (char*)block + sizeof(Block);
		fBlock = block;
	}

	SkASSERT(fBlock && bytes <= fBlock->fFreeSize);
	void* ptr = fBlock->fFreePtr;

	fBlock->fFreeSize -= bytes;
	fBlock->fFreePtr += bytes;
	return ptr;
}

