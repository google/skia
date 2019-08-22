
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkIStream_DEFINED
#define SkIStream_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "src/core/SkLeanWindows.h"
#include "src/utils/win/SkObjBase.h"
#include <ole2.h>

class SkStream;
class SkWStream;

/**
 * A bare IStream implementation which properly reference counts
 * but returns E_NOTIMPL for all ISequentialStream and IStream methods.
 */
class SkBaseIStream : public IStream {
public:
    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject) override;
    SK_STDMETHODIMP_(ULONG) AddRef() override;
    SK_STDMETHODIMP_(ULONG) Release() override;

    // ISequentialStream methods
    SK_STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override;
    SK_STDMETHODIMP Write(void const* pv, ULONG cb, ULONG* pcbWritten) override;

    // IStream methods
    SK_STDMETHODIMP SetSize(ULARGE_INTEGER) override;
    SK_STDMETHODIMP CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override;
    SK_STDMETHODIMP Commit(DWORD) override;
    SK_STDMETHODIMP Revert() override;
    SK_STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    SK_STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    SK_STDMETHODIMP Clone(IStream**) override;
    SK_STDMETHODIMP Seek(LARGE_INTEGER liDistanceToMove,
                         DWORD dwOrigin,
                         ULARGE_INTEGER* lpNewFilePointer) override;
    SK_STDMETHODIMP Stat(STATSTG* pStatstg, DWORD grfStatFlag) override;

protected:
    explicit SkBaseIStream();
    virtual ~SkBaseIStream();

private:
    LONG _refcount;
};

/**
 * A minimal read-only IStream implementation which wraps an SkStream.
 */
class SkIStream : public SkBaseIStream {
public:
    HRESULT static CreateFromSkStream(std::unique_ptr<SkStreamAsset>, IStream** ppStream);

    SK_STDMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) override;
    SK_STDMETHODIMP Write(void const* pv, ULONG cb, ULONG* pcbWritten) override;
    SK_STDMETHODIMP Seek(LARGE_INTEGER liDistanceToMove,
                         DWORD dwOrigin,
                         ULARGE_INTEGER* lpNewFilePointer) override;
    SK_STDMETHODIMP Stat(STATSTG* pStatstg, DWORD grfStatFlag) override;

private:
    const std::unique_ptr<SkStream> fSkStream;
    ULARGE_INTEGER fLocation;

    explicit SkIStream(std::unique_ptr<SkStreamAsset>);
    ~SkIStream() override;
};

/**
 * A minimal write-only IStream implementation which wraps an SkWIStream.
 */
class SkWIStream : public SkBaseIStream {
public:
    HRESULT static CreateFromSkWStream(SkWStream* stream, IStream ** ppStream);

    SK_STDMETHODIMP Write(void const* pv, ULONG cb, ULONG* pcbWritten) override;
    SK_STDMETHODIMP Commit(DWORD) override;
    SK_STDMETHODIMP Stat(STATSTG* pStatstg, DWORD grfStatFlag) override;

private:
    SkWStream *fSkWStream;

    SkWIStream(SkWStream* stream);
    ~SkWIStream() override;
};

#endif  // SK_BUILD_FOR_WIN
#endif  // SkIStream_DEFINED
