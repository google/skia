/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "SkDWriteFontFileStream.h"
#include "SkHRESULT.h"

#include <dwrite.h>
#include <limits>

///////////////////////////////////////////////////////////////////////////////
//  SkIDWriteFontFileStream

SkDWriteFontFileStream::SkDWriteFontFileStream(IDWriteFontFileStream* fontFileStream)
    : fFontFileStream(fontFileStream)
    , fPos(0)
    , fLockedMemory(NULL)
    , fFragmentLock(NULL) {
    fontFileStream->AddRef();
}

SkDWriteFontFileStream::~SkDWriteFontFileStream() {
    if (fFragmentLock) {
        fFontFileStream->ReleaseFileFragment(fFragmentLock);
    }
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

bool SkDWriteFontFileStream::rewind() {
    fPos = 0;
    return true;
}

size_t SkDWriteFontFileStream::read(void* buffer, size_t size) {
    HRESULT hr = S_OK;

    if (NULL == buffer) {
        UINT64 realFileSize = 0;
        hr = fFontFileStream->GetFileSize(&realFileSize);
        if (realFileSize > (std::numeric_limits<size_t>::max)()) {
            return 0;
        }
        size_t fileSize = static_cast<size_t>(realFileSize);
        if (size == 0) {
            return fileSize;
        } else {
            if (fPos + size > fileSize) {
                size_t skipped = fileSize - fPos;
                fPos = fileSize;
                return skipped;
            } else {
                fPos += size;
                return size;
            }
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
    UINT64 realFileSize = 0;
    hr = fFontFileStream->GetFileSize(&realFileSize);
    if (realFileSize > (std::numeric_limits<size_t>::max)()) {
        return 0;
    }
    size_t fileSize = static_cast<size_t>(realFileSize);
    if (fPos + size > fileSize) {
        size_t read = fileSize - fPos;
        hr = fFontFileStream->ReadFileFragment(&start, fPos, read, &fragmentLock);
        if (SUCCEEDED(hr)) {
            memcpy(buffer, start, read);
            fFontFileStream->ReleaseFileFragment(fragmentLock);
            fPos = fileSize;
            return read;
        }
        return 0;
    } else {
        //This means we were within bounds, but failed for some other reason.
        return 0;
    }
}


///////////////////////////////////////////////////////////////////////////////
//  SkIDWriteFontFileStreamWrapper

HRESULT SkDWriteFontFileStreamWrapper::Create(SkStream* stream, SkDWriteFontFileStreamWrapper** streamFontFileStream) {
    *streamFontFileStream = new SkDWriteFontFileStreamWrapper(stream);
    if (NULL == streamFontFileStream) {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

SkDWriteFontFileStreamWrapper::SkDWriteFontFileStreamWrapper(SkStream* stream)
    : fRefCount(1), fStream(stream) {
    stream->ref();
}

HRESULT STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileStream)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = NULL;
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
        *fragmentStart = NULL;
        *fragmentContext = NULL;
        return E_FAIL;
    }

    if (fileOffset + fragmentSize > (std::numeric_limits<size_t>::max)()) {
        return E_FAIL;
    }

    const void* data = fStream->getMemoryBase();
    if (NULL != data) {
        *fragmentStart = static_cast<BYTE const*>(data) + static_cast<size_t>(fileOffset);
        *fragmentContext = NULL;

    } else {
        //May be called from multiple threads.
        SkAutoMutexAcquire ama(fStreamMutex);

        *fragmentStart = NULL;
        *fragmentContext = NULL;

        if (!fStream->rewind()) {
            return E_FAIL;
        }
        if (fStream->skip(static_cast<size_t>(fileOffset)) != fileOffset) {
            return E_FAIL;
        }
        SkAutoTDeleteArray<uint8_t> streamData(new uint8_t[static_cast<size_t>(fragmentSize)]);
        if (fStream->read(streamData.get(), static_cast<size_t>(fragmentSize)) != fragmentSize) {
            return E_FAIL;
        }

        *fragmentStart = streamData.get();
        *fragmentContext = streamData.detach();
    }
    return S_OK;
}

void STDMETHODCALLTYPE SkDWriteFontFileStreamWrapper::ReleaseFileFragment(void* fragmentContext) {
    if (NULL == fragmentContext) {
        return;
    }
    delete [] fragmentContext;
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
