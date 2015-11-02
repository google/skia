/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "SkTypes.h"
#include "SkDWriteFontFileStream.h"
#include "SkHRESULT.h"
#include "SkTemplates.h"
#include "SkTFitsIn.h"
#include "SkTScopedComPtr.h"

#include <dwrite.h>

///////////////////////////////////////////////////////////////////////////////
//  SkIDWriteFontFileStream

SkDWriteFontFileStream::SkDWriteFontFileStream(IDWriteFontFileStream* fontFileStream)
    : fFontFileStream(SkRefComPtr(fontFileStream))
    , fPos(0)
    , fLockedMemory(nullptr)
    , fFragmentLock(nullptr) {
}

SkDWriteFontFileStream::~SkDWriteFontFileStream() {
    if (fFragmentLock) {
        fFontFileStream->ReleaseFileFragment(fFragmentLock);
    }
}

size_t SkDWriteFontFileStream::read(void* buffer, size_t size) {
    HRESULT hr = S_OK;

    if (nullptr == buffer) {
        size_t fileSize = this->getLength();

        if (fPos + size > fileSize) {
            size_t skipped = fileSize - fPos;
            fPos = fileSize;
            return skipped;
        } else {
            fPos += size;
            return size;
        }
    }

    const void* start;
    void* fragmentLock;
    hr = fFontFileStream->ReadFileFragment(&start, fPos, size, &fragmentLock);
    if (SUCCEEDED(hr)) {
        memcpy(buffer, start, size);
        fFontFileStream->ReleaseFileFragment(fragmentLock);
        fPos += size;
        return size;
    }

    //The read may have failed because we asked for too much data.
    size_t fileSize = this->getLength();
    if (fPos + size <= fileSize) {
        //This means we were within bounds, but failed for some other reason.
        return 0;
    }

    size_t read = fileSize - fPos;
    hr = fFontFileStream->ReadFileFragment(&start, fPos, read, &fragmentLock);
    if (SUCCEEDED(hr)) {
        memcpy(buffer, start, read);
        fFontFileStream->ReleaseFileFragment(fragmentLock);
        fPos = fileSize;
        return read;
    }

    return 0;
}

bool SkDWriteFontFileStream::isAtEnd() const {
    return fPos == this->getLength();
}

bool SkDWriteFontFileStream::rewind() {
    fPos = 0;
    return true;
}

SkDWriteFontFileStream* SkDWriteFontFileStream::duplicate() const {
    return new SkDWriteFontFileStream(fFontFileStream.get());
}

size_t SkDWriteFontFileStream::getPosition() const {
    return fPos;
}

bool SkDWriteFontFileStream::seek(size_t position) {
    size_t length = this->getLength();
    fPos = (position > length) ? length : position;
    return true;
}

bool SkDWriteFontFileStream::move(long offset) {
    return seek(fPos + offset);
}

SkDWriteFontFileStream* SkDWriteFontFileStream::fork() const {
    SkAutoTDelete<SkDWriteFontFileStream> that(this->duplicate());
    that->seek(fPos);
    return that.detach();
}

size_t SkDWriteFontFileStream::getLength() const {
    HRESULT hr = S_OK;
    UINT64 realFileSize = 0;
    hr = fFontFileStream->GetFileSize(&realFileSize);
    if (!SkTFitsIn<size_t>(realFileSize)) {
        return 0;
    }
    return static_cast<size_t>(realFileSize);
}

const void* SkDWriteFontFileStream::getMemoryBase() {
    if (fLockedMemory) {
        return fLockedMemory;
    }

    UINT64 fileSize;
    HRNM(fFontFileStream->GetFileSize(&fileSize), "Could not get file size");
    HRNM(fFontFileStream->ReadFileFragment(&fLockedMemory, 0, fileSize, &fFragmentLock),
         "Could not lock file fragment.");
    return fLockedMemory;
}

///////////////////////////////////////////////////////////////////////////////
//  SkIDWriteFontFileStreamWrapper

HRESULT SkDWriteFontFileStreamWrapper::Create(SkStreamAsset* stream,
                                              SkDWriteFontFileStreamWrapper** streamFontFileStream)
{
    *streamFontFileStream = new SkDWriteFontFileStreamWrapper(stream);
    if (nullptr == streamFontFileStream) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

SkDWriteFontFileStreamWrapper::SkDWriteFontFileStreamWrapper(SkStreamAsset* stream)
    : fRefCount(1), fStream(stream) {
}

HRESULT STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileStream)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

ULONG STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

HRESULT STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::ReadFileFragment(
    void const** fragmentStart,
    UINT64 fileOffset,
    UINT64 fragmentSize,
    void** fragmentContext)
{
    // The loader is responsible for doing a bounds check.
    UINT64 fileSize;
    this->GetFileSize(&fileSize);
    if (fileOffset > fileSize || fragmentSize > fileSize - fileOffset) {
        *fragmentStart = nullptr;
        *fragmentContext = nullptr;
        return E_FAIL;
    }

    if (!SkTFitsIn<size_t>(fileOffset + fragmentSize)) {
        return E_FAIL;
    }

    const void* data = fStream->getMemoryBase();
    if (data) {
        *fragmentStart = static_cast<BYTE const*>(data) + static_cast<size_t>(fileOffset);
        *fragmentContext = nullptr;

    } else {
        // May be called from multiple threads.
        SkAutoMutexAcquire ama(fStreamMutex);

        *fragmentStart = nullptr;
        *fragmentContext = nullptr;

        if (!fStream->seek(static_cast<size_t>(fileOffset))) {
            return E_FAIL;
        }
        SkAutoTMalloc<uint8_t> streamData(static_cast<size_t>(fragmentSize));
        if (fStream->read(streamData.get(), static_cast<size_t>(fragmentSize)) != fragmentSize) {
            return E_FAIL;
        }

        *fragmentStart = streamData.get();
        *fragmentContext = streamData.detach();
    }
    return S_OK;
}

void STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::ReleaseFileFragment(void* fragmentContext) {
    sk_free(fragmentContext);
}

HRESULT STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::GetFileSize(UINT64* fileSize) {
    *fileSize = fStream->getLength();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::GetLastWriteTime(UINT64* lastWriteTime) {
    // The concept of last write time does not apply to this loader.
    *lastWriteTime = 0;
    return E_NOTIMPL;
}

#endif//defined(SK_BUILD_FOR_WIN32)
