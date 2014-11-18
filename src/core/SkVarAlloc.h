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
        fRemaining -= bytes;
        return ptr;
    }

private:
    void makeSpace(size_t bytes, unsigned flags);

    char* fByte;
    unsigned fRemaining;
    unsigned fLgSize;  // This is always in the range [4, 16], so it really only needs 4 bits.

    struct Block;
    Block* fBlock;
};

#endif//SkVarAlloc_DEFINED
