/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDWriteFontFileStream_DEFINED
#define SkDWriteFontFileStream_DEFINED

#include "include/core/SkTypes.h"

#include "include/core/SkStream.h"
#include "include/private/SkMutex.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>

/**
 *  An SkStream backed by an IDWriteFontFileStream.
 *  This allows Skia code to read an IDWriteFontFileStream.
 */
class SkDWriteFontFileStream : public SkStreamMemory {
public:
    explicit SkDWriteFontFileStream(IDWriteFontFileStream* fontFileStream);
    ~SkDWriteFontFileStream() override;

    size_t read(void* buffer, size_t size) override;
    bool isAtEnd() const override;
    bool rewind() override;
    size_t getPosition() const override;
    bool seek(size_t position) override;
    bool move(long offset) override;
    size_t getLength() const override;
    const void* getMemoryBase() override;

    std::unique_ptr<SkDWriteFontFileStream> duplicate() const {
        return std::unique_ptr<SkDWriteFontFileStream>(this->onDuplicate());
    }
    std::unique_ptr<SkDWriteFontFileStream> fork() const {
        return std::unique_ptr<SkDWriteFontFileStream>(this->onFork());
    }

private:
    SkDWriteFontFileStream* onDuplicate() const override;
    SkDWriteFontFileStream* onFork() const override;

    SkTScopedComPtr<IDWriteFontFileStream> fFontFileStream;
    size_t fPos;
    const void* fLockedMemory;
    void* fFragmentLock;
};

/**
 *  An IDWriteFontFileStream backed by an SkStream.
 *  This allows DirectWrite to read an SkStream.
 */
class SkDWriteFontFileStreamWrapper : public IDWriteFontFileStream {
public:
    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileStream methods
    virtual HRESULT STDMETHODCALLTYPE ReadFileFragment(
        void const** fragmentStart,
        UINT64 fileOffset,
        UINT64 fragmentSize,
        void** fragmentContext);

    virtual void STDMETHODCALLTYPE ReleaseFileFragment(void* fragmentContext);
    virtual HRESULT STDMETHODCALLTYPE GetFileSize(UINT64* fileSize);
    virtual HRESULT STDMETHODCALLTYPE GetLastWriteTime(UINT64* lastWriteTime);

    static HRESULT Create(SkStreamAsset* stream,
                          SkDWriteFontFileStreamWrapper** streamFontFileStream);

private:
    explicit SkDWriteFontFileStreamWrapper(SkStreamAsset* stream);
    virtual ~SkDWriteFontFileStreamWrapper() { }

    ULONG fRefCount;
    std::unique_ptr<SkStreamAsset> fStream;
    SkMutex fStreamMutex;
};
#endif
