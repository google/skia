#include "SkWriter32.h"

struct SkWriter32::Block {
    Block*  fNext;
    size_t  fSize;
    size_t  fAllocated;
    
    size_t  available() const { return fSize - fAllocated; }
    char*   base() { return (char*)(this + 1); }
    const char* base() const { return (const char*)(this + 1); }
    
    uint32_t* alloc(size_t size)
    {
        SkASSERT(SkAlign4(size) == size);
        SkASSERT(this->available() >= size);
        void* ptr = this->base() + fAllocated;
        fAllocated += size;
        SkASSERT(fAllocated <= fSize);
        return (uint32_t*)ptr;
    }
    
    uint32_t* peek32(size_t offset)
    {
        SkASSERT(offset <= fAllocated + 4);
        void* ptr = this->base() + offset;
        return (uint32_t*)ptr;
    }

    static Block* Create(size_t size)
    {
        SkASSERT(SkAlign4(size) == size);
        Block* block = (Block*)sk_malloc_throw(sizeof(Block) + size);
        block->fNext = NULL;
        block->fSize = size;
        block->fAllocated = 0;
        return block;
    }
};

///////////////////////////////////////////////////////////////////////////////

SkWriter32::~SkWriter32()
{
    this->reset();
}

void SkWriter32::reset()
{
    Block* block = fHead;    
    while (block)
    {
        Block* next = block->fNext;
        sk_free(block);
        block = next;
    }
    fHead = fTail = NULL;
    fSize = 0;
}

uint32_t* SkWriter32::reserve(size_t size)
{
    SkASSERT(SkAlign4(size) == size);
    
    Block* block = fTail;

    if (NULL == block)
    {
        SkASSERT(NULL == fHead);
        fHead = fTail = block = Block::Create(SkMax32(size, fMinSize));
    }
    else if (block->available() < size)
    {
        fTail = Block::Create(SkMax32(size, fMinSize));
        block->fNext = fTail;
        block = fTail;
    }
    
    fSize += size;

    return block->alloc(size);
}

uint32_t* SkWriter32::peek32(size_t offset)
{
    SkASSERT(SkAlign4(offset) == offset);
    SkASSERT(offset <= fSize);

    Block* block = fHead;
    SkASSERT(NULL != block);
    
    while (offset >= block->fAllocated)
    {
        offset -= block->fAllocated;
        block = block->fNext;
        SkASSERT(NULL != block);
    }
    return block->peek32(offset);
}

void SkWriter32::flatten(void* dst) const
{
    const Block* block = fHead;
    SkDEBUGCODE(size_t total = 0;)

    while (block)
    {
        size_t allocated = block->fAllocated;
        memcpy(dst, block->base(), allocated);
        dst = (char*)dst + allocated;
        block = block->fNext;

        SkDEBUGCODE(total += allocated;)
        SkASSERT(total <= fSize);
    }
    SkASSERT(total == fSize);
}

void SkWriter32::writePad(const void* src, size_t size) {
    size_t alignedSize = SkAlign4(size);
    char* dst = (char*)this->reserve(alignedSize);
    memcpy(dst, src, size);
    dst += size;
    int n = alignedSize - size;
    while (--n >= 0) {
        *dst++ = 0;
    }
}

#include "SkStream.h"

size_t SkWriter32::readFromStream(SkStream* stream, size_t length) {
    char scratch[1024];
    const size_t MAX = sizeof(scratch);
    size_t remaining = length;
    
    while (remaining != 0) {
        size_t n = remaining;
        if (n > MAX) {
            n = MAX;
        }
        size_t bytes = stream->read(scratch, n);
        this->writePad(scratch, bytes);
        remaining -= bytes;
        if (bytes != n) {
            break;
        }
    }
    return length - remaining;
}

bool SkWriter32::writeToStream(SkWStream* stream) {
    const Block* block = fHead;    
    while (block) {
        if (!stream->write(block->base(), block->fAllocated)) {
            return false;
        }
        block = block->fNext;
    }
    return true;
}

