/* libs/graphics/images/SkStream.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkStream.h"
#include "SkFixed.h"
#include "SkString.h"
#include "SkOSFile.h"

const char* SkStream::getFileName()
{
    // override in subclass if you represent a file
    return NULL;
}

static const char gPrefix[] = "file:\0" "http:\0" "https:\0" "ftp:\0"  ;

bool SkStream::IsAbsoluteURI(const char path[])
{
    int prefix = SkStrStartsWithOneOf(path, gPrefix);
    if (prefix >= 0)
        return true;
    return path[0] != '.';
}

SkStream* SkStream::GetURIStream(const char prefix[], const char path[])
{
    int index = SkStrStartsWithOneOf(path, gPrefix);
    if (prefix && prefix[0] && index < 0)
    {
        SkString fullPath(prefix);
        fullPath.append(path);
        return GetURIStream(NULL, fullPath.c_str());
    }
    if (index > 0)
      return SkNEW_ARGS(SkURLStream, (path));
    if (index == 0)
        path += 5;
    return SkNEW_ARGS(SkFILEStream, (path));    
}

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

bool SkWStream::writeDecAsText(S32 dec)
{
    SkString    tmp;
    tmp.appendS32(dec);
    return this->write(tmp.c_str(), tmp.size());
}

bool SkWStream::writeHexAsText(U32 hex, int digits)
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

////////////////////////////////////////////////////////////////////////////

SkFILEStream::SkFILEStream(const char file[]) : fName(file)
{
#ifdef SK_BUILD_FOR_BREW
    if (SkStrEndsWith(fName.c_str(), ".xml"))
        fName.writable_str()[fName.size()-3] = 'b';
#endif

    fFILE = file ? sk_fopen(fName.c_str(), kRead_SkFILE_Flag) : NULL;
}

SkFILEStream::~SkFILEStream()
{
    if (fFILE)
        sk_fclose(fFILE);
}

void SkFILEStream::setPath(const char path[])
{
    fName.set(path);
#ifdef SK_BUILD_FOR_BREW
    if (SkStrEndsWith(fName.c_str(), ".xml"))
        fName.writable_str()[fName.size()-3] = 'b';
#endif

    if (fFILE)
    {
        sk_fclose(fFILE);
        fFILE = NULL;
    }
    if (path)
        fFILE = sk_fopen(fName.c_str(), kRead_SkFILE_Flag);
}

const char* SkFILEStream::getFileName()
{
    return fName.c_str();
}

bool SkFILEStream::rewind()
{
    if (fFILE)
    {
        if (sk_frewind(fFILE))
            return true;
        // we hit an error
        sk_fclose(fFILE);
        fFILE = NULL;
    }
    return false;
}

size_t SkFILEStream::read(void* buffer, size_t size)
{
    if (fFILE)
    {
#if 1
        if (buffer == NULL && size == 0)    // funny, they want the size I think
            return sk_fgetsize(fFILE);
        return sk_fread(buffer, size, fFILE);
#else
        if (buffer)
        {
            size_t bytes = sk_fread(buffer, size, fFILE);
            if (ferror(fFILE) == 0)
                return bytes;
        }
        else
        {
            if (size == 0 && ::fseek(fFILE, 0, SEEK_END) == 0) 
            {
                size = ::ftell(fFILE);
                ::fseek(fFILE, 0, SEEK_SET);
                return size;
            } 
            else if (::fseek(fFILE, (long)size, SEEK_CUR) == 0)
                return size;
        }
#endif
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////

SkMemoryStream::SkMemoryStream(const void* src, size_t size)
    : fSrc(src), fSize(size)
{
    fOffset = 0;
}

bool SkMemoryStream::rewind()
{
    fOffset = 0;
    return true;
}

size_t SkMemoryStream::read(void* buffer, size_t size)
{
    // if buffer is NULL, seek ahead by size

    if (size == 0)
        return 0;
    if (size > fSize - fOffset)
        size = fSize - fOffset;
    if (buffer) {
        memcpy(buffer, (const char*)fSrc + fOffset, size);
    }
    fOffset += size;
    return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

SkBufferStream::SkBufferStream(SkStream& proxy, size_t bufferSize)
    : fProxy(proxy)
{
    SkASSERT(&proxy != NULL);
    this->init(NULL, bufferSize);
}

SkBufferStream::SkBufferStream(SkStream& proxy, void* buffer, size_t bufferSize)
    : fProxy(proxy)
{
    SkASSERT(&proxy != NULL);
    SkASSERT(buffer == NULL || bufferSize != 0);    // init(addr, 0) makes no sense, we must know how big their buffer is

    this->init(buffer, bufferSize);
}

void SkBufferStream::init(void* buffer, size_t bufferSize)
{
    if (bufferSize == 0)
        bufferSize = kDefaultBufferSize;

    fOrigBufferSize = bufferSize;
    fBufferSize = bufferSize;
    fBufferOffset = bufferSize; // to trigger a reload on the first read()

    if (buffer == NULL)
    {
        fBuffer = (char*)sk_malloc_throw(fBufferSize);
        fWeOwnTheBuffer = true;
    }
    else
    {
        fBuffer = (char*)buffer;
        fWeOwnTheBuffer = false;
    }
}

SkBufferStream::~SkBufferStream()
{
    if (fWeOwnTheBuffer)
        sk_free(fBuffer);
}

bool SkBufferStream::rewind()
{
    fBufferOffset = fBufferSize = fOrigBufferSize;
    return fProxy.rewind();
}

const char* SkBufferStream::getFileName()
{
    return fProxy.getFileName();
}

#ifdef SK_DEBUG
//  #define SK_TRACE_BUFFERSTREAM
#endif

size_t SkBufferStream::read(void* buffer, size_t size)
{
#ifdef SK_TRACE_BUFFERSTREAM
    SkDebugf("Request %d", size);
#endif
    if (buffer == NULL || size == 0)
    {
        fBufferOffset += size;
        return fProxy.read(buffer, size);
    }

    size_t s = size;
    size_t actuallyRead = 0;

    // flush what we can from our fBuffer
    if (fBufferOffset < fBufferSize)
    {
        if (s > fBufferSize - fBufferOffset)
            s = fBufferSize - fBufferOffset;
        memcpy(buffer, fBuffer + fBufferOffset, s);
#ifdef SK_TRACE_BUFFERSTREAM
        SkDebugf(" flush %d", s);
#endif
        size -= s;
        fBufferOffset += s;
        buffer = (char*)buffer + s;
        actuallyRead = s;
    }

    // check if there is more to read
    if (size)
    {
        SkASSERT(fBufferOffset >= fBufferSize); // need to refill our fBuffer

        if (size < fBufferSize) // lets try to read more than the request
        {
            s = fProxy.read(fBuffer, fBufferSize);
#ifdef SK_TRACE_BUFFERSTREAM
            SkDebugf(" read %d into fBuffer", s);
#endif
            if (size > s)   // they asked for too much
                size = s;
            if (size)
            {
                memcpy(buffer, fBuffer, size);
                actuallyRead += size;
#ifdef SK_TRACE_BUFFERSTREAM
                SkDebugf(" memcpy %d into dst", size);
#endif
            }

            fBufferOffset = size;
            fBufferSize = s;        // record the (possibly smaller) size for the buffer
        }
        else    // just do a direct read
        {
            actuallyRead += fProxy.read(buffer, size);
#ifdef SK_TRACE_BUFFERSTREAM
            SkDebugf(" direct read %d", size);
#endif
        }
    }
#ifdef SK_TRACE_BUFFERSTREAM
    SkDebugf("\n");
#endif
    return actuallyRead;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

SkFILEWStream::SkFILEWStream(const char path[])
{
    fFILE = sk_fopen(path, kWrite_SkFILE_Flag);
}

SkFILEWStream::~SkFILEWStream()
{
    if (fFILE)
        sk_fclose(fFILE);
}

bool SkFILEWStream::write(const void* buffer, size_t size)
{
    if (fFILE == NULL)
        return false;

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
    if (fFILE)
        sk_fflush(fFILE);
}

////////////////////////////////////////////////////////////////////////

SkMemoryWStream::SkMemoryWStream(void* buffer, size_t size)
    : fBuffer((char*)buffer), fMaxLength(size), fBytesWritten(0)
{
}

bool SkMemoryWStream::write(const void* buffer, size_t size)
{
    size = SkMin32(size, fMaxLength - fBytesWritten);
    if (size > 0)
    {
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

SkDynamicMemoryWStream::SkDynamicMemoryWStream() : fHead(NULL), fTail(NULL), fBytesWritten(0), fCopyToCache(NULL)
{
}

SkDynamicMemoryWStream::~SkDynamicMemoryWStream()
{
    sk_free(fCopyToCache);

    Block*  block = fHead;
    
    while (block != NULL) {
        Block*  next = block->fNext;
        sk_free(block);
        block = next;
    }
}

bool SkDynamicMemoryWStream::write(const void* buffer, size_t count)
{
    if (count > 0) {

        if (fCopyToCache) {
            sk_free(fCopyToCache);
            fCopyToCache = NULL;
        }
        fBytesWritten += count;
        
        size_t  size;
        
        if (fTail != NULL && fTail->avail() > 0) {
            size = SkMin32(fTail->avail(), count);
            buffer = fTail->append(buffer, size);
            SkASSERT(count >= size);
            count -= size;        
            if (count == 0)
                return true;
        }
            
        size = SkMax32(count, SkDynamicMemoryWStream_MinBlockSize);
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
    if (offset + count > fBytesWritten)
        return false; // test does not partially modify
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

void SkDynamicMemoryWStream::copyTo(void* dst) const
{
    Block* block = fHead;
    
    while (block != NULL) {
        size_t size = block->written();
        memcpy(dst, block->start(), size);
        dst = (void*)((char*)dst + size);
        block = block->fNext;
    }
}

const char* SkDynamicMemoryWStream::getStream() const
{
    if (fCopyToCache == NULL) {
        fCopyToCache = (char*)sk_malloc_throw(fBytesWritten);
        this->copyTo(fCopyToCache);
    }
    return fCopyToCache;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void SkDebugWStream::newline()
{
#ifdef SK_DEBUG
    SkDebugf("\n");
#endif
}

bool SkDebugWStream::write(const void* buffer, size_t size)
{
#ifdef SK_DEBUG
    char* s = new char[size+1];
    memcpy(s, buffer, size);
    s[size] = 0;
    SkDebugf("%s", s);
    delete[] s;
#endif
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#include "SkRandom.h"

void SkWStream::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    {
        static const char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        char            copy[sizeof(s)];
        SkRandom        rand;

        for (int i = 0; i < 65; i++)
        {
            char*           copyPtr = copy;
            SkMemoryStream  mem(s, sizeof(s));
            SkBufferStream  buff(mem, i);

            do {
                copyPtr += buff.read(copyPtr, rand.nextU() & 15);
            } while (copyPtr < copy + sizeof(s));
            SkASSERT(copyPtr == copy + sizeof(s));
            SkASSERT(memcmp(s, copy, sizeof(s)) == 0);
        }
    }
    {
        SkDebugWStream  s;

        s.writeText("testing wstream helpers\n");
        s.writeText("compare: 0 ");         s.writeDecAsText(0);    s.newline();
        s.writeText("compare: 591 ");       s.writeDecAsText(591);  s.newline();
        s.writeText("compare: -9125 ");     s.writeDecAsText(-9125);    s.newline();
        s.writeText("compare: 0 ");         s.writeHexAsText(0, 0); s.newline();
        s.writeText("compare: 03FA ");      s.writeHexAsText(0x3FA, 4); s.newline();
        s.writeText("compare: DEADBEEF ");  s.writeHexAsText(0xDEADBEEF, 4);    s.newline();
        s.writeText("compare: 0 ");         s.writeScalarAsText(SkIntToScalar(0));  s.newline();
        s.writeText("compare: 27 ");        s.writeScalarAsText(SkIntToScalar(27)); s.newline();
        s.writeText("compare: -119 ");      s.writeScalarAsText(SkIntToScalar(-119));   s.newline();
        s.writeText("compare: 851.3333 ");  s.writeScalarAsText(SkIntToScalar(851) + SK_Scalar1/3); s.newline();
        s.writeText("compare: -0.08 ");     s.writeScalarAsText(-SK_Scalar1*8/100); s.newline();
    }

    {
        SkDynamicMemoryWStream  ds;
        const char s[] = "abcdefghijklmnopqrstuvwxyz";
        int i;
        for (i = 0; i < 100; i++) {
            bool result = ds.write(s, 26);
            SkASSERT(result);
        }
        SkASSERT(ds.getOffset() == 100 * 26);
        char* dst = new char[100 * 26 + 1];
        dst[100*26] = '*';
        ds.copyTo(dst);
        SkASSERT(dst[100*26] == '*');
   //     char* p = dst;
        for (i = 0; i < 100; i++)
            SkASSERT(memcmp(&dst[i * 26], s, 26) == 0);
        SkASSERT(memcmp(dst, ds.getStream(), 100*26) == 0);
        delete[] dst;
    }
#endif
}

#endif
