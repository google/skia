/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBuffer_DEFINED
#define SkBuffer_DEFINED

#include "SkScalar.h"
#include "SkTypes.h"

/** \class SkRBuffer

    Light weight class for reading data from a memory block.
    The RBuffer is given the buffer to read from, with either a specified size
    or no size (in which case no range checking is performed). It is iillegal
    to attempt to read a value from an empty RBuffer (data == null).
*/
class SkRBuffer : SkNoncopyable {
public:
    SkRBuffer() : fData(0), fPos(0), fStop(0) {}
    /** Initialize RBuffer with a data pointer, but no specified length.
        This signals the RBuffer to not perform range checks during reading.
    */
    SkRBuffer(const void* data) {
        fData = (const char*)data;
        fPos = (const char*)data;
        fStop = 0;  // no bounds checking
    }
    /** Initialize RBuffer with a data point and length.
    */
    SkRBuffer(const void* data, size_t size) {
        SkASSERT(data != 0 || size == 0);
        fData = (const char*)data;
        fPos = (const char*)data;
        fStop = (const char*)data + size;
    }

    virtual ~SkRBuffer() { }

    /** Return the number of bytes that have been read from the beginning
        of the data pointer.
    */
    size_t  pos() const { return fPos - fData; }
    /** Return the total size of the data pointer. Only defined if the length was
        specified in the constructor or in a call to reset().
    */
    size_t  size() const { return fStop - fData; }
    /** Return true if the buffer has read to the end of the data pointer.
        Only defined if the length was specified in the constructor or in a call
        to reset(). Always returns true if the length was not specified.
    */
    bool    eof() const { return fPos >= fStop; }

    /** Read the specified number of bytes from the data pointer. If buffer is not
        null, copy those bytes into buffer.
    */
    virtual bool read(void* buffer, size_t size) {
        if (size) {
            this->readNoSizeCheck(buffer, size);
        }
        return true;
    }

    const void* skip(size_t size); // return start of skipped data
    size_t  skipToAlign4();

    bool readPtr(void** ptr) { return read(ptr, sizeof(void*)); }
    bool readScalar(SkScalar* x) { return read(x, 4); }
    bool readU32(uint32_t* x) { return read(x, 4); }
    bool readS32(int32_t* x) { return read(x, 4); }
    bool readU16(uint16_t* x) { return read(x, 2); }
    bool readS16(int16_t* x) { return read(x, 2); }
    bool readU8(uint8_t* x) { return read(x, 1); }
    bool readBool(bool* x) {
        uint8_t u8;
        if (this->readU8(&u8)) {
            *x = (u8 != 0);
            return true;
        }
        return false;
    }

protected:
    void    readNoSizeCheck(void* buffer, size_t size);

    const char* fData;
    const char* fPos;
    const char* fStop;
};

/** \class SkRBufferWithSizeCheck

    Same as SkRBuffer, except that a size check is performed before the read operation and an
    error is set if the read operation is attempting to read past the end of the data.
*/
class SkRBufferWithSizeCheck : public SkRBuffer {
public:
    SkRBufferWithSizeCheck(const void* data, size_t size) : SkRBuffer(data, size), fError(false) {}

    /** Read the specified number of bytes from the data pointer. If buffer is not
        null and the number of bytes to read does not overflow this object's data,
        copy those bytes into buffer.
    */
    bool read(void* buffer, size_t size) override;

    /** Returns whether or not a read operation attempted to read past the end of the data.
    */
    bool isValid() const { return !fError; }
private:
    bool fError;
};

/** \class SkWBuffer

    Light weight class for writing data to a memory block.
    The WBuffer is given the buffer to write into, with either a specified size
    or no size, in which case no range checking is performed. An empty WBuffer
    is legal, in which case no data is ever written, but the relative pos()
    is updated.
*/
class SkWBuffer : SkNoncopyable {
public:
    SkWBuffer() : fData(0), fPos(0), fStop(0) {}
    SkWBuffer(void* data) { reset(data); }
    SkWBuffer(void* data, size_t size) { reset(data, size); }

    void reset(void* data) {
        fData = (char*)data;
        fPos = (char*)data;
        fStop = 0;  // no bounds checking
    }

    void reset(void* data, size_t size) {
        SkASSERT(data != 0 || size == 0);
        fData = (char*)data;
        fPos = (char*)data;
        fStop = (char*)data + size;
    }

    size_t  pos() const { return fPos - fData; }
    void*   skip(size_t size); // return start of skipped data

    void write(const void* buffer, size_t size) {
        if (size) {
            this->writeNoSizeCheck(buffer, size);
        }
    }

    size_t  padToAlign4();

    void    writePtr(const void* x) { this->writeNoSizeCheck(&x, sizeof(x)); }
    void    writeScalar(SkScalar x) { this->writeNoSizeCheck(&x, 4); }
    void    write32(int32_t x) { this->writeNoSizeCheck(&x, 4); }
    void    write16(int16_t x) { this->writeNoSizeCheck(&x, 2); }
    void    write8(int8_t x) { this->writeNoSizeCheck(&x, 1); }
    void    writeBool(bool x) { this->write8(x); }

private:
    void    writeNoSizeCheck(const void* buffer, size_t size);

    char* fData;
    char* fPos;
    char* fStop;
};

#endif
