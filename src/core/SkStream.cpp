
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkData.h"
#include "SkFixed.h"
#include "SkString.h"
#include "SkOSFile.h"
#include "SkTypes.h"

///////////////////////////////////////////////////////////////////////////////


int8_t SkStream::readS8() {
    int8_t value;
    SkDEBUGCODE(size_t len =) this->read(&value, 1);
    SkASSERT(1 == len);
    return value;
}

int16_t SkStream::readS16() {
    int16_t value;
    SkDEBUGCODE(size_t len =) this->read(&value, 2);
    SkASSERT(2 == len);
    return value;
}

int32_t SkStream::readS32() {
    int32_t value;
    SkDEBUGCODE(size_t len =) this->read(&value, 4);
    SkASSERT(4 == len);
    return value;
}

SkScalar SkStream::readScalar() {
    SkScalar value;
    SkDEBUGCODE(size_t len =) this->read(&value, sizeof(SkScalar));
    SkASSERT(sizeof(SkScalar) == len);
    return value;
}

#define SK_MAX_BYTE_FOR_U8          0xFD
#define SK_BYTE_SENTINEL_FOR_U16    0xFE
#define SK_BYTE_SENTINEL_FOR_U32    0xFF

size_t SkStream::readPackedUInt() {
    uint8_t byte;
    if (!this->read(&byte, 1)) {
        return 0;
    }
    if (SK_BYTE_SENTINEL_FOR_U16 == byte) {
        return this->readU16();
    } else if (SK_BYTE_SENTINEL_FOR_U32 == byte) {
        return this->readU32();
    } else {
        return byte;
    }
}

//////////////////////////////////////////////////////////////////////////////////////

SkWStream::~SkWStream()
{
}

void SkWStream::newline()
{
    this->write("\n", 1);
}

void SkWStream::flush()
{
}

bool SkWStream::writeText(const char text[])
{
    SkASSERT(text);
    return this->write(text, strlen(text));
}

bool SkWStream::writeDecAsText(int32_t dec)
{
    SkString    tmp;
    tmp.appendS32(dec);
    return this->write(tmp.c_str(), tmp.size());
}

bool SkWStream::writeBigDecAsText(int64_t dec, int minDigits)
{
    SkString    tmp;
    tmp.appendS64(dec, minDigits);
    return this->write(tmp.c_str(), tmp.size());
}

bool SkWStream::writeHexAsText(uint32_t hex, int digits)
{
    SkString    tmp;
    tmp.appendHex(hex, digits);
    return this->write(tmp.c_str(), tmp.size());
}

bool SkWStream::writeScalarAsText(SkScalar value)
{
    SkString    tmp;
    tmp.appendScalar(value);
    return this->write(tmp.c_str(), tmp.size());
}

bool SkWStream::write8(U8CPU value) {
    uint8_t v = SkToU8(value);
    return this->write(&v, 1);
}

bool SkWStream::write16(U16CPU value) {
    uint16_t v = SkToU16(value);
    return this->write(&v, 2);
}

bool SkWStream::write32(uint32_t value) {
    return this->write(&value, 4);
}

bool SkWStream::writeScalar(SkScalar value) {
    return this->write(&value, sizeof(value));
}

int SkWStream::SizeOfPackedUInt(size_t value) {
    if (value <= SK_MAX_BYTE_FOR_U8) {
        return 1;
    } else if (value <= 0xFFFF) {
        return 3;
    }
    return 5;
}

bool SkWStream::writePackedUInt(size_t value) {
    uint8_t data[5];
    size_t len = 1;
    if (value <= SK_MAX_BYTE_FOR_U8) {
        data[0] = value;
        len = 1;
    } else if (value <= 0xFFFF) {
        uint16_t value16 = value;
        data[0] = SK_BYTE_SENTINEL_FOR_U16;
        memcpy(&data[1], &value16, 2);
        len = 3;
    } else {
        uint32_t value32 = SkToU32(value);
        data[0] = SK_BYTE_SENTINEL_FOR_U32;
        memcpy(&data[1], &value32, 4);
        len = 5;
    }
    return this->write(data, len);
}

bool SkWStream::writeStream(SkStream* stream, size_t length) {
    char scratch[1024];
    const size_t MAX = sizeof(scratch);

    while (length != 0) {
        size_t n = length;
        if (n > MAX) {
            n = MAX;
        }
        stream->read(scratch, n);
        if (!this->write(scratch, n)) {
            return false;
        }
        length -= n;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkFILEStream::SkFILEStream(const char file[]) : fName(file), fOwnership(kCallerPasses_Ownership) {
    fFILE = file ? sk_fopen(fName.c_str(), kRead_SkFILE_Flag) : NULL;
}

SkFILEStream::SkFILEStream(FILE* file, Ownership ownership)
    : fFILE((SkFILE*)file)
    , fOwnership(ownership) {
}

SkFILEStream::~SkFILEStream() {
    if (fFILE && fOwnership != kCallerRetains_Ownership) {
        sk_fclose(fFILE);
    }
}

void SkFILEStream::setPath(const char path[]) {
    fName.set(path);
    if (fFILE) {
        sk_fclose(fFILE);
        fFILE = NULL;
    }
    if (path) {
        fFILE = sk_fopen(fName.c_str(), kRead_SkFILE_Flag);
    }
}

size_t SkFILEStream::read(void* buffer, size_t size) {
    if (fFILE) {
        return sk_fread(buffer, size, fFILE);
    }
    return 0;
}

bool SkFILEStream::isAtEnd() const {
    return sk_feof(fFILE);
}

bool SkFILEStream::rewind() {
    if (fFILE) {
        if (sk_frewind(fFILE)) {
            return true;
        }
        // we hit an error
        sk_fclose(fFILE);
        fFILE = NULL;
    }
    return false;
}

SkStreamAsset* SkFILEStream::duplicate() const {
    if (NULL == fFILE) {
        return new SkMemoryStream();
    }

    if (fData.get()) {
        return new SkMemoryStream(fData);
    }

    if (!fName.isEmpty()) {
        SkAutoTUnref<SkFILEStream> that(new SkFILEStream(fName.c_str()));
        if (sk_fidentical(that->fFILE, this->fFILE)) {
            return that.detach();
        }
    }

    fData.reset(SkData::NewFromFILE(fFILE));
    if (NULL == fData.get()) {
        return NULL;
    }
    return new SkMemoryStream(fData);
}

size_t SkFILEStream::getPosition() const {
    return sk_ftell(fFILE);
}

bool SkFILEStream::seek(size_t position) {
    return sk_fseek(fFILE, position);
}

bool SkFILEStream::move(long offset) {
    return sk_fmove(fFILE, offset);
}

SkStreamAsset* SkFILEStream::fork() const {
    SkAutoTUnref<SkStreamAsset> that(this->duplicate());
    that->seek(this->getPosition());
    return that.detach();
}

size_t SkFILEStream::getLength() const {
    return sk_fgetsize(fFILE);
}

const void* SkFILEStream::getMemoryBase() {
    if (NULL == fData.get()) {
        return NULL;
    }
    return fData->data();
}

///////////////////////////////////////////////////////////////////////////////

static SkData* newFromParams(const void* src, size_t size, bool copyData) {
    if (copyData) {
        return SkData::NewWithCopy(src, size);
    } else {
        return SkData::NewWithoutCopy(src, size);
    }
}

SkMemoryStream::SkMemoryStream() {
    fData = SkData::NewEmpty();
    fOffset = 0;
}

SkMemoryStream::SkMemoryStream(size_t size) {
    fData = SkData::NewUninitialized(size);
    fOffset = 0;
}

SkMemoryStream::SkMemoryStream(const void* src, size_t size, bool copyData) {
    fData = newFromParams(src, size, copyData);
    fOffset = 0;
}

SkMemoryStream::SkMemoryStream(SkData* data) {
    if (NULL == data) {
        fData = SkData::NewEmpty();
    } else {
        fData = data;
        fData->ref();
    }
    fOffset = 0;
}

SkMemoryStream::~SkMemoryStream() {
    fData->unref();
}

void SkMemoryStream::setMemoryOwned(const void* src, size_t size) {
    fData->unref();
    fData = SkData::NewFromMalloc(src, size);
    fOffset = 0;
}

void SkMemoryStream::setMemory(const void* src, size_t size, bool copyData) {
    fData->unref();
    fData = newFromParams(src, size, copyData);
    fOffset = 0;
}

SkData* SkMemoryStream::copyToData() const {
    fData->ref();
    return fData;
}

SkData* SkMemoryStream::setData(SkData* data) {
    fData->unref();
    if (NULL == data) {
        fData = SkData::NewEmpty();
    } else {
        fData = data;
        fData->ref();
    }
    fOffset = 0;
    return data;
}

void SkMemoryStream::skipToAlign4() {
    // cast to remove unary-minus warning
    fOffset += -(int)fOffset & 0x03;
}

size_t SkMemoryStream::read(void* buffer, size_t size) {
    size_t dataSize = fData->size();

    if (size > dataSize - fOffset) {
        size = dataSize - fOffset;
    }
    if (buffer) {
        memcpy(buffer, fData->bytes() + fOffset, size);
    }
    fOffset += size;
    return size;
}

bool SkMemoryStream::isAtEnd() const {
    return fOffset == fData->size();
}

bool SkMemoryStream::rewind() {
    fOffset = 0;
    return true;
}

SkMemoryStream* SkMemoryStream::duplicate() const {
    return SkNEW_ARGS(SkMemoryStream, (fData));
}

size_t SkMemoryStream::getPosition() const {
    return fOffset;
}

bool SkMemoryStream::seek(size_t position) {
    fOffset = position > fData->size()
            ? fData->size()
            : position;
    return true;
}

bool SkMemoryStream::move(long offset) {
    return this->seek(fOffset + offset);
}

SkMemoryStream* SkMemoryStream::fork() const {
    SkAutoTUnref<SkMemoryStream> that(this->duplicate());
    that->seek(fOffset);
    return that.detach();
}

size_t SkMemoryStream::getLength() const {
    return fData->size();
}

const void* SkMemoryStream::getMemoryBase() {
    return fData->data();
}

const void* SkMemoryStream::getAtPos() {
    return fData->bytes() + fOffset;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

SkFILEWStream::SkFILEWStream(const char path[])
{
    fFILE = sk_fopen(path, kWrite_SkFILE_Flag);
}

SkFILEWStream::~SkFILEWStream()
{
    if (fFILE) {
        sk_fclose(fFILE);
    }
}

size_t SkFILEWStream::bytesWritten() const {
    return sk_ftell(fFILE);
}

bool SkFILEWStream::write(const void* buffer, size_t size)
{
    if (fFILE == NULL) {
        return false;
    }

    if (sk_fwrite(buffer, size, fFILE) != size)
    {
        SkDEBUGCODE(SkDebugf("SkFILEWStream failed writing %d bytes\n", size);)
        sk_fclose(fFILE);
        fFILE = NULL;
        return false;
    }
    return true;
}

void SkFILEWStream::flush()
{
    if (fFILE) {
        sk_fflush(fFILE);
    }
}

////////////////////////////////////////////////////////////////////////

SkMemoryWStream::SkMemoryWStream(void* buffer, size_t size)
    : fBuffer((char*)buffer), fMaxLength(size), fBytesWritten(0)
{
}

bool SkMemoryWStream::write(const void* buffer, size_t size) {
    size = SkTMin(size, fMaxLength - fBytesWritten);
    if (size > 0) {
        memcpy(fBuffer + fBytesWritten, buffer, size);
        fBytesWritten += size;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////

#define SkDynamicMemoryWStream_MinBlockSize   256

struct SkDynamicMemoryWStream::Block {
    Block*  fNext;
    char*   fCurr;
    char*   fStop;

    const char* start() const { return (const char*)(this + 1); }
    char*   start() { return (char*)(this + 1); }
    size_t  avail() const { return fStop - fCurr; }
    size_t  written() const { return fCurr - this->start(); }

    void init(size_t size)
    {
        fNext = NULL;
        fCurr = this->start();
        fStop = this->start() + size;
    }

    const void* append(const void* data, size_t size)
    {
        SkASSERT((size_t)(fStop - fCurr) >= size);
        memcpy(fCurr, data, size);
        fCurr += size;
        return (const void*)((const char*)data + size);
    }
};

SkDynamicMemoryWStream::SkDynamicMemoryWStream()
    : fHead(NULL), fTail(NULL), fBytesWritten(0), fCopy(NULL)
{
}

SkDynamicMemoryWStream::~SkDynamicMemoryWStream()
{
    reset();
}

void SkDynamicMemoryWStream::reset()
{
    this->invalidateCopy();

    Block*  block = fHead;

    while (block != NULL) {
        Block*  next = block->fNext;
        sk_free(block);
        block = next;
    }
    fHead = fTail = NULL;
    fBytesWritten = 0;
}

bool SkDynamicMemoryWStream::write(const void* buffer, size_t count)
{
    if (count > 0) {
        this->invalidateCopy();

        fBytesWritten += count;

        size_t  size;

        if (fTail != NULL && fTail->avail() > 0) {
            size = SkTMin(fTail->avail(), count);
            buffer = fTail->append(buffer, size);
            SkASSERT(count >= size);
            count -= size;
            if (count == 0)
                return true;
        }

        size = SkTMax<size_t>(count, SkDynamicMemoryWStream_MinBlockSize);
        Block* block = (Block*)sk_malloc_throw(sizeof(Block) + size);
        block->init(size);
        block->append(buffer, count);

        if (fTail != NULL)
            fTail->fNext = block;
        else
            fHead = fTail = block;
        fTail = block;
    }
    return true;
}

bool SkDynamicMemoryWStream::write(const void* buffer, size_t offset, size_t count)
{
    if (offset + count > fBytesWritten) {
        return false; // test does not partially modify
    }

    this->invalidateCopy();

    Block* block = fHead;
    while (block != NULL) {
        size_t size = block->written();
        if (offset < size) {
            size_t part = offset + count > size ? size - offset : count;
            memcpy(block->start() + offset, buffer, part);
            if (count <= part)
                return true;
            count -= part;
            buffer = (const void*) ((char* ) buffer + part);
        }
        offset = offset > size ? offset - size : 0;
        block = block->fNext;
    }
    return false;
}

bool SkDynamicMemoryWStream::read(void* buffer, size_t offset, size_t count)
{
    if (offset + count > fBytesWritten)
        return false; // test does not partially modify
    Block* block = fHead;
    while (block != NULL) {
        size_t size = block->written();
        if (offset < size) {
            size_t part = offset + count > size ? size - offset : count;
            memcpy(buffer, block->start() + offset, part);
            if (count <= part)
                return true;
            count -= part;
            buffer = (void*) ((char* ) buffer + part);
        }
        offset = offset > size ? offset - size : 0;
        block = block->fNext;
    }
    return false;
}

void SkDynamicMemoryWStream::copyTo(void* dst) const
{
    if (fCopy) {
        memcpy(dst, fCopy->data(), fBytesWritten);
    } else {
        Block* block = fHead;

        while (block != NULL) {
            size_t size = block->written();
            memcpy(dst, block->start(), size);
            dst = (void*)((char*)dst + size);
            block = block->fNext;
        }
    }
}

void SkDynamicMemoryWStream::padToAlign4()
{
    // cast to remove unary-minus warning
    int padBytes = -(int)fBytesWritten & 0x03;
    if (padBytes == 0)
        return;
    int zero = 0;
    write(&zero, padBytes);
}

SkData* SkDynamicMemoryWStream::copyToData() const {
    if (NULL == fCopy) {
        SkData* data = SkData::NewUninitialized(fBytesWritten);
        // be sure to call copyTo() before we assign to fCopy
        this->copyTo(data->writable_data());
        fCopy = data;
    }
    return SkRef(fCopy);
}

void SkDynamicMemoryWStream::invalidateCopy() {
    if (fCopy) {
        fCopy->unref();
        fCopy = NULL;
    }
}

class SkBlockMemoryRefCnt : public SkRefCnt {
public:
    explicit SkBlockMemoryRefCnt(SkDynamicMemoryWStream::Block* head) : fHead(head) { }

    virtual ~SkBlockMemoryRefCnt() {
        SkDynamicMemoryWStream::Block* block = fHead;
        while (block != NULL) {
            SkDynamicMemoryWStream::Block* next = block->fNext;
            sk_free(block);
            block = next;
        }
    }

    SkDynamicMemoryWStream::Block* const fHead;
};

class SkBlockMemoryStream : public SkStreamAsset {
public:
    SkBlockMemoryStream(SkDynamicMemoryWStream::Block* head, size_t size)
        : fBlockMemory(SkNEW_ARGS(SkBlockMemoryRefCnt, (head))), fCurrent(head)
        , fSize(size) , fOffset(0), fCurrentOffset(0) { }

    SkBlockMemoryStream(SkBlockMemoryRefCnt* headRef, size_t size)
        : fBlockMemory(SkRef(headRef)), fCurrent(fBlockMemory->fHead)
        , fSize(size) , fOffset(0), fCurrentOffset(0) { }

    virtual size_t read(void* buffer, size_t rawCount) SK_OVERRIDE {
        size_t count = rawCount;
        if (fOffset + count > fSize) {
            count = fSize - fOffset;
        }
        size_t bytesLeftToRead = count;
        while (fCurrent != NULL) {
            size_t bytesLeftInCurrent = fCurrent->written() - fCurrentOffset;
            size_t bytesFromCurrent = SkTMin(bytesLeftToRead, bytesLeftInCurrent);
            if (buffer) {
                memcpy(buffer, fCurrent->start() + fCurrentOffset, bytesFromCurrent);
                buffer = SkTAddOffset<void>(buffer, bytesFromCurrent);
            }
            if (bytesLeftToRead <= bytesFromCurrent) {
                fCurrentOffset += bytesFromCurrent;
                fOffset += count;
                return count;
            }
            bytesLeftToRead -= bytesFromCurrent;
            fCurrent = fCurrent->fNext;
            fCurrentOffset = 0;
        }
        SkASSERT(false);
        return 0;
    }

    virtual bool isAtEnd() const SK_OVERRIDE {
        return fOffset == fSize;
    }

    virtual bool rewind() SK_OVERRIDE {
        fCurrent = fBlockMemory->fHead;
        fOffset = 0;
        fCurrentOffset = 0;
        return true;
    }

    virtual SkBlockMemoryStream* duplicate() const SK_OVERRIDE {
        return SkNEW_ARGS(SkBlockMemoryStream, (fBlockMemory.get(), fSize));
    }

    virtual size_t getPosition() const SK_OVERRIDE {
        return fOffset;
    }

    virtual bool seek(size_t position) SK_OVERRIDE {
        // If possible, skip forward.
        if (position >= fOffset) {
            size_t skipAmount = position - fOffset;
            return this->skip(skipAmount) == skipAmount;
        }
        // If possible, move backward within the current block.
        size_t moveBackAmount = fOffset - position;
        if (moveBackAmount <= fCurrentOffset) {
            fCurrentOffset -= moveBackAmount;
            fOffset -= moveBackAmount;
            return true;
        }
        // Otherwise rewind and move forward.
        return this->rewind() && this->skip(position) == position;
    }

    virtual bool move(long offset) SK_OVERRIDE {
        return seek(fOffset + offset);
    }

    virtual SkBlockMemoryStream* fork() const SK_OVERRIDE {
        SkAutoTUnref<SkBlockMemoryStream> that(this->duplicate());
        that->fCurrent = this->fCurrent;
        that->fOffset = this->fOffset;
        that->fCurrentOffset = this->fCurrentOffset;
        return that.detach();
    }

    virtual size_t getLength() const SK_OVERRIDE {
        return fSize;
    }

    virtual const void* getMemoryBase() SK_OVERRIDE {
        if (NULL == fBlockMemory->fHead->fNext) {
            return fBlockMemory->fHead->start();
        }
        return NULL;
    }

private:
    SkAutoTUnref<SkBlockMemoryRefCnt> const fBlockMemory;
    SkDynamicMemoryWStream::Block const * fCurrent;
    size_t const fSize;
    size_t fOffset;
    size_t fCurrentOffset;
};

SkStreamAsset* SkDynamicMemoryWStream::detachAsStream() {
    if (fCopy) {
        SkMemoryStream* stream = SkNEW_ARGS(SkMemoryStream, (fCopy));
        this->reset();
        return stream;
    }
    SkBlockMemoryStream* stream = SkNEW_ARGS(SkBlockMemoryStream, (fHead, fBytesWritten));
    fHead = 0;
    this->reset();
    return stream;
}

///////////////////////////////////////////////////////////////////////////////

void SkDebugWStream::newline()
{
#if defined(SK_DEBUG) || defined(SK_DEVELOPER)
    SkDebugf("\n");
    fBytesWritten++;
#endif
}

bool SkDebugWStream::write(const void* buffer, size_t size)
{
#if defined(SK_DEBUG) || defined(SK_DEVELOPER)
    char* s = new char[size+1];
    memcpy(s, buffer, size);
    s[size] = 0;
    SkDebugf("%s", s);
    delete[] s;
    fBytesWritten += size;
#endif
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static SkData* mmap_filename(const char path[]) {
    SkFILE* file = sk_fopen(path, kRead_SkFILE_Flag);
    if (NULL == file) {
        return NULL;
    }

    SkData* data = SkData::NewFromFILE(file);
    sk_fclose(file);
    return data;
}

SkStreamAsset* SkStream::NewFromFile(const char path[]) {
    SkAutoTUnref<SkData> data(mmap_filename(path));
    if (data.get()) {
        return SkNEW_ARGS(SkMemoryStream, (data.get()));
    }

    // If we get here, then our attempt at using mmap failed, so try normal
    // file access.
    SkFILEStream* stream = SkNEW_ARGS(SkFILEStream, (path));
    if (!stream->isValid()) {
        stream->unref();
        stream = NULL;
    }
    return stream;
}

// Declared in SkStreamPriv.h:
size_t SkCopyStreamToStorage(SkAutoMalloc* storage, SkStream* stream) {
    SkASSERT(storage != NULL);
    SkASSERT(stream != NULL);

    if (stream->hasLength()) {
        const size_t length = stream->getLength();
        void* dst = storage->reset(length);
        if (stream->read(dst, length) != length) {
            return 0;
        }
        return length;
    }

    SkDynamicMemoryWStream tempStream;
    // Arbitrary buffer size.
    const size_t bufferSize = 256 * 1024; // 256KB
    char buffer[bufferSize];
    SkDEBUGCODE(size_t debugLength = 0;)
    do {
        size_t bytesRead = stream->read(buffer, bufferSize);
        tempStream.write(buffer, bytesRead);
        SkDEBUGCODE(debugLength += bytesRead);
        SkASSERT(tempStream.bytesWritten() == debugLength);
    } while (!stream->isAtEnd());
    const size_t length = tempStream.bytesWritten();
    void* dst = storage->reset(length);
    tempStream.copyTo(dst);
    return length;
}

// Declared in SkStreamPriv.h:
SkData* SkCopyStreamToData(SkStream* stream) {
    SkASSERT(stream != NULL);

    if (stream->hasLength()) {
        return SkData::NewFromStream(stream, stream->getLength());
    }

    SkDynamicMemoryWStream tempStream;
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    do {
        size_t bytesRead = stream->read(buffer, bufferSize);
        tempStream.write(buffer, bytesRead);
    } while (!stream->isAtEnd());
    return tempStream.copyToData();
}

SkStreamRewindable* SkStreamRewindableFromSkStream(SkStream* stream) {
    if (!stream) {
        return NULL;
    }
    SkAutoTUnref<SkStreamRewindable> dupStream(stream->duplicate());
    if (dupStream) {
        return dupStream.detach();
    }
    stream->rewind();
    if (stream->hasLength()) {
        size_t length = stream->getLength();
        if (stream->hasPosition()) {  // If stream has length, but can't rewind.
            length -= stream->getPosition();
        }
        SkAutoTUnref<SkData> data(SkData::NewFromStream(stream, length));
        return SkNEW_ARGS(SkMemoryStream, (data.get()));
    }
    SkDynamicMemoryWStream tempStream;
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    do {
        size_t bytesRead = stream->read(buffer, bufferSize);
        tempStream.write(buffer, bytesRead);
    } while (!stream->isAtEnd());
    return tempStream.detachAsStream();  // returns a SkBlockMemoryStream,
                                         // cheaper than copying to SkData
}
