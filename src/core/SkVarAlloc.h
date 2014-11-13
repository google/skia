#ifndef SkVarAlloc_DEFINED
#define SkVarAlloc_DEFINED

#include "SkTypes.h"

class SkVarAlloc : SkNoncopyable {
public:
    // SkVarAlloc will never allocate less than smallest bytes at a time.
    // When it allocates a new block, it will be at least growth times bigger than the last.
    SkVarAlloc(size_t smallest, float growth);
    ~SkVarAlloc();

    // Returns contiguous bytes aligned at least for pointers.  You may pass SK_MALLOC_THROW, etc.
    char* alloc(size_t bytes, unsigned sk_malloc_flags) {
        bytes = SkAlignPtr(bytes);

        if (fByte + bytes > fLimit) {
            this->makeSpace(bytes, sk_malloc_flags);
        }
        SkASSERT(fByte + bytes <= fLimit);

        char* ptr = fByte;
        fByte += bytes;
        return ptr;
    }

private:
    void makeSpace(size_t bytes, unsigned flags);

    char* fByte;
    const char* fLimit;

    unsigned fSmallest;
    const float fGrowth;

    struct Block;
    Block* fBlock;
};

#endif//SkVarAlloc_DEFINED
