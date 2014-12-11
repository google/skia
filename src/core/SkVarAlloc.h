#ifndef SkVarAlloc_DEFINED
#define SkVarAlloc_DEFINED

#include "SkTypes.h"

class SkVarAlloc : SkNoncopyable {
public:
    SkVarAlloc();
    ~SkVarAlloc();

    // Returns contiguous bytes aligned at least for pointers.  You may pass SK_MALLOC_THROW, etc.
    char* alloc(size_t bytes, unsigned sk_malloc_flags) {
        bytes = SkAlignPtr(bytes);

        if (bytes > fRemaining) {
            this->makeSpace(bytes, sk_malloc_flags);
        }
        SkASSERT(bytes <= fRemaining);

        char* ptr = fByte;
        fByte += bytes;
        fRemaining = SkToU32(fRemaining - bytes);
        return ptr;
    }

    // Returns our best estimate of the number of bytes we've allocated.
    // (We intentionally do not track this precisely to save space.)
    size_t approxBytesAllocated() const;

private:
    void makeSpace(size_t bytes, unsigned flags);

    char* fByte;
    unsigned fRemaining;
    unsigned fLgSize;  // This is always in the range [4, 16], so it really only needs 4 bits.

    struct Block;
    Block* fBlock;
};
SK_COMPILE_ASSERT(sizeof(SkVarAlloc) <= 24, SkVarAllocSize);

#endif//SkVarAlloc_DEFINED
