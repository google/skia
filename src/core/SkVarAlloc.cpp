#include "SkVarAlloc.h"

// We use non-standard malloc diagnostic methods to make sure our allocations are sized well.
#if defined(SK_BUILD_FOR_MAC)
    #include <malloc/malloc.h>
#elif defined(SK_BUILD_FOR_LINUX)
    #include <malloc.h>
#endif

struct SkVarAlloc::Block {
    Block* prev;
    char* data() { return (char*)(this + 1); }

    static Block* Alloc(Block* prev, size_t size, unsigned flags) {
        SkASSERT(size >= sizeof(Block));
        Block* b = (Block*)sk_malloc_flags(size, flags);
        b->prev = prev;
        return b;
    }
};

SkVarAlloc::SkVarAlloc()
    : fByte(NULL)
    , fRemaining(0)
    , fLgMinSize(4)
    , fBlock(NULL) {}

SkVarAlloc::~SkVarAlloc() {
    Block* b = fBlock;
    while (b) {
        Block* prev = b->prev;
        sk_free(b);
        b = prev;
    }
}

void SkVarAlloc::makeSpace(size_t bytes, unsigned flags) {
    SkASSERT(SkIsAlignPtr(bytes));

    size_t alloc = 1<<(fLgMinSize++);
    while (alloc < bytes + sizeof(Block)) {
        alloc *= 2;
    }
    fBlock = Block::Alloc(fBlock, alloc, flags);
    fByte = fBlock->data();
    fRemaining = alloc - sizeof(Block);

#if defined(SK_BUILD_FOR_MAC)
    SkASSERT(alloc == malloc_good_size(alloc));
#elif defined(SK_BUILD_FOR_LINUX)
    SkASSERT(alloc == malloc_usable_size(fByte - sizeof(Block)));
#endif
}
