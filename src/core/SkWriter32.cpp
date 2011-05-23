#include "SkWriter32.h"

struct SkWriter32::Block {
    Block*  fNext;
    size_t  fSize;
    size_t  fAllocated;
    
    size_t  available() const { return fSize - fAllocated; }
    char*   base() { return (char*)(this + 1); }
    const char* base() const { return (const char*)(this + 1); }
    
    uint32_t* alloc(size_t size) {
        SkASSERT(SkAlign4(size) == size);
        SkASSERT(this->available() >= size);
        void* ptr = this->base() + fAllocated;
        fAllocated += size;
        SkASSERT(fAllocated <= fSize);
        return (uint32_t*)ptr;
    }
    
    uint32_t* peek32(size_t offset) {
        SkASSERT(offset <= fAllocated + 4);
        void* ptr = this->base() + offset;
        return (uint32_t*)ptr;
    }

    static Block* Create(size_t size) {
        SkASSERT(SkAlign4(size) == size);
        Block* block = (Block*)sk_malloc_throw(sizeof(Block) + size);
        block->fNext = NULL;
        block->fSize = size;
        block->fAllocated = 0;
        return block;
    }
};

///////////////////////////////////////////////////////////////////////////////

SkWriter32::~SkWriter32() {
    this->reset();
}

void SkWriter32::reset() {
    Block* block = fHead;    
    while (block) {
        Block* next = block->fNext;
        sk_free(block);
        block = next;
    }

    fSize = 0;
    fHead = fTail = NULL;
    fSingleBlock = NULL;
}

void SkWriter32::reset(void* block, size_t size) {
    this->reset();
    SkASSERT(0 == ((fSingleBlock - (char*)0) & 3));   // need 4-byte alignment
    fSingleBlock = (char*)block;
    fSingleBlockSize = (size & ~3);
}

uint32_t* SkWriter32::reserve(size_t size) {
    SkASSERT(SkAlign4(size) == size);

    if (fSingleBlock) {
        uint32_t* ptr = (uint32_t*)(fSingleBlock + fSize);
        fSize += size;
        SkASSERT(fSize <= fSingleBlockSize);
        return ptr;
    }

    Block* block = fTail;

    if (NULL == block) {
        SkASSERT(NULL == fHead);
        fHead = fTail = block = Block::Create(SkMax32(size, fMinSize));
    } else if (block->available() < size) {
        fTail = Block::Create(SkMax32(size, fMinSize));
        block->fNext = fTail;
        block = fTail;
    }
    
    fSize += size;

    return block->alloc(size);
}

uint32_t* SkWriter32::peek32(size_t offset) {
    SkASSERT(SkAlign4(offset) == offset);
    SkASSERT(offset <= fSize);

    if (fSingleBlock) {
        return (uint32_t*)(fSingleBlock + offset);
    }

    Block* block = fHead;
    SkASSERT(NULL != block);
    
    while (offset >= block->fAllocated) {
        offset -= block->fAllocated;
        block = block->fNext;
        SkASSERT(NULL != block);
    }
    return block->peek32(offset);
}

void SkWriter32::flatten(void* dst) const {
    if (fSingleBlock) {
        memcpy(dst, fSingleBlock, fSize);
        return;
    }

    const Block* block = fHead;
    SkDEBUGCODE(size_t total = 0;)

    while (block) {
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
    if (fSingleBlock) {
        SkASSERT(fSingleBlockSize >= fSize);
        size_t remaining = fSingleBlockSize - fSize;
        if (length > remaining) {
            length = remaining;
        }
        stream->read(fSingleBlock + fSize, length);
        fSize += length;
        return length;
    }

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
    if (fSingleBlock) {
        return stream->write(fSingleBlock, fSize);
    }

    const Block* block = fHead;    
    while (block) {
        if (!stream->write(block->base(), block->fAllocated)) {
            return false;
        }
        block = block->fNext;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkReader32.h"

const char* SkReader32::readString(size_t* outLen) {
    // we need to read at least 1-4 bytes
    SkASSERT(this->isAvailable(4));
    const uint8_t* base = (const uint8_t*)this->peek();
    const uint8_t* ptr = base;

    size_t len = *ptr++;
    if (0xFF == len) {
        len = (ptr[0] << 8) | ptr[1];
        ptr += 2;
        SkASSERT(len < 0xFFFF);
    }
    
    // skip what we've read, and 0..3 pad bytes
    // add 1 for the terminating 0 that writeString() included
    size_t alignedSize = SkAlign4(len + (ptr - base) + 1);
    this->skip(alignedSize);

    if (outLen) {
        *outLen = len;
    }
    return (const char*)ptr;
}

void SkWriter32::writeString(const char str[], size_t len) {
    if ((long)len < 0) {
        SkASSERT(str);
        len = strlen(str);
    }
    size_t lenBytes = 1;
    if (len >= 0xFF) {
        lenBytes = 3;
        SkASSERT(len < 0xFFFF);
    }
    // add 1 since we also write a terminating 0
    size_t alignedLen = SkAlign4(lenBytes + len + 1);
    uint8_t* ptr = (uint8_t*)this->reserve(alignedLen);
    if (1 == lenBytes) {
        *ptr++ = SkToU8(len);
    } else {
        *ptr++ = 0xFF;
        *ptr++ = SkToU8(len >> 8);
        *ptr++ = len & 0xFF;
    }
    memcpy(ptr, str, len);
    ptr[len] = 0;
    // we may have left 0,1,2,3 bytes uninitialized, since we reserved align4
    // number of bytes. That's ok, since the reader will know to skip those
}

size_t SkWriter32::WriteStringSize(const char* str, size_t len) {
    if ((long)len < 0) {
        SkASSERT(str);
        len = strlen(str);
    }
    size_t lenBytes = 1;
    if (len >= 0xFF) {
        lenBytes = 3;
        SkASSERT(len < 0xFFFF);
    }
    // add 1 since we also write a terminating 0
    return SkAlign4(lenBytes + len + 1);
}


