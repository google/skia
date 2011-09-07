
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkReader32_DEFINED
#define SkReader32_DEFINED

#include "SkScalar.h"

class SkString;

class SkReader32 : SkNoncopyable {
public:
    SkReader32() : fCurr(NULL), fStop(NULL), fBase(NULL) {}
    SkReader32(const void* data, size_t size)  {
        this->setMemory(data, size);
    }

    void setMemory(const void* data, size_t size) {
        SkASSERT(ptr_align_4(data));
        SkASSERT(SkAlign4(size) == size);
        
        fBase = fCurr = (const char*)data;
        fStop = (const char*)data + size;
    }
    
    uint32_t size() const { return fStop - fBase; }
    uint32_t offset() const { return fCurr - fBase; }
    bool eof() const { return fCurr >= fStop; }
    const void* base() const { return fBase; }
    const void* peek() const { return fCurr; }

    uint32_t available() const { return fStop - fCurr; }
    bool isAvailable(uint32_t size) const { return fCurr + size <= fStop; }
    
    void rewind() { fCurr = fBase; }

    void setOffset(size_t offset) {
        SkASSERT(SkAlign4(offset) == offset);
        SkASSERT(offset <= this->size());
        fCurr = fBase + offset;
    }
    
    bool readBool() { return this->readInt() != 0; }
    
    int32_t readInt() {
        SkASSERT(ptr_align_4(fCurr));
        int32_t value = *(const int32_t*)fCurr;
        fCurr += sizeof(value);
        SkASSERT(fCurr <= fStop);
        return value;
    }
    
    SkScalar readScalar() {
        SkASSERT(ptr_align_4(fCurr));
        SkScalar value = *(const SkScalar*)fCurr;
        fCurr += sizeof(value);
        SkASSERT(fCurr <= fStop);
        return value;
    }
    
    const void* skip(size_t size) {
        SkASSERT(ptr_align_4(fCurr));
        const void* addr = fCurr;
        fCurr += SkAlign4(size);
        SkASSERT(fCurr <= fStop);
        return addr;
    }
    
    template <typename T> const T& skipT() {
        SkASSERT(SkAlign4(sizeof(T)) == sizeof(T));
        return *(const T*)this->skip(sizeof(T));
    }

    void read(void* dst, size_t size) {
        SkASSERT(0 == size || dst != NULL);
        SkASSERT(ptr_align_4(fCurr));
        memcpy(dst, fCurr, size);
        fCurr += SkAlign4(size);
        SkASSERT(fCurr <= fStop);
    }
    
    uint8_t readU8() { return (uint8_t)this->readInt(); }
    uint16_t readU16() { return (uint16_t)this->readInt(); }
    int32_t readS32() { return this->readInt(); }
    uint32_t readU32() { return this->readInt(); }

    /**
     *  Read the length of a string (written by SkWriter32::writeString) into
     *  len (if len is not NULL) and return the null-ternimated address of the
     *  string within the reader's buffer.
     */
    const char* readString(size_t* len = NULL);

    /**
     *  Read the string (written by SkWriter32::writeString) and return it in
     *  copy (if copy is not null). Return the length of the string.
     */
    size_t readIntoString(SkString* copy);

private:
    // these are always 4-byte aligned
    const char* fCurr;  // current position within buffer
    const char* fStop;  // end of buffer
    const char* fBase;  // beginning of buffer
    
#ifdef SK_DEBUG
    static bool ptr_align_4(const void* ptr) {
        return (((const char*)ptr - (const char*)NULL) & 3) == 0;
    }
#endif
};

#endif
