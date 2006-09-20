#ifndef SkChunkAlloc_DEFINED
#define SkChunkAlloc_DEFINED

#include "SkTypes.h"

class SkChunkAlloc {
public:
	SkChunkAlloc(size_t minSize) : fBlock(nil), fMinSize(SkAlign4(minSize)) {}
	~SkChunkAlloc();

	void	reset();

	enum AllocFailType {
		kReturnNil_AllocFailType,
		kThrow_AllocFailType
	};
	void*	alloc(size_t bytes, AllocFailType);
	
private:
	struct Block {
		Block*	fNext;
		size_t	fFreeSize;
		char*	fFreePtr;
		// data[] follows
	};
	Block*	fBlock;
	size_t	fMinSize;
};

#endif
