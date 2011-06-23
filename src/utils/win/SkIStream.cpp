/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ole2.h>
#include "SkIStream.h"
#include "SkStream.h"

/**
 * SkBaseIStream
 */
SkBaseIStream::SkBaseIStream() : _refcount(1) { }
SkBaseIStream::~SkBaseIStream() { }

HRESULT STDMETHODCALLTYPE SkBaseIStream::QueryInterface(REFIID iid
                                                      , void ** ppvObject)
{
    if (NULL == ppvObject) {
        return E_INVALIDARG;
    }
    if (iid == __uuidof(IUnknown)
        || iid == __uuidof(IStream)
        || iid == __uuidof(ISequentialStream))
    {
        *ppvObject = static_cast<IStream*>(this);
        AddRef();
        return S_OK;
    } else {
        *ppvObject = NULL;
        return E_NOINTERFACE; 
    }
}

ULONG STDMETHODCALLTYPE SkBaseIStream::AddRef(void) {
    return (ULONG)InterlockedIncrement(&_refcount);
}

ULONG STDMETHODCALLTYPE SkBaseIStream::Release(void) {
    ULONG res = (ULONG) InterlockedDecrement(&_refcount);
    if (0 == res) {
        delete this;
    }
    return res;
}

// ISequentialStream Interface
HRESULT STDMETHODCALLTYPE SkBaseIStream::Read(void* pv
                                            , ULONG cb
                                            , ULONG* pcbRead)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Write(void const* pv
                                             , ULONG cb
                                             , ULONG* pcbWritten)
{ return E_NOTIMPL; }

// IStream Interface
HRESULT STDMETHODCALLTYPE SkBaseIStream::SetSize(ULARGE_INTEGER) 
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::CopyTo(IStream*
                                              , ULARGE_INTEGER
                                              , ULARGE_INTEGER*
                                              , ULARGE_INTEGER*)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Commit(DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Revert(void)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::LockRegion(ULARGE_INTEGER
                                                  , ULARGE_INTEGER
                                                  , DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::UnlockRegion(ULARGE_INTEGER
                                                    , ULARGE_INTEGER
                                                    , DWORD)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Clone(IStream **)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Seek(LARGE_INTEGER liDistanceToMove
                                            , DWORD dwOrigin
                                            , ULARGE_INTEGER* lpNewFilePointer)
{ return E_NOTIMPL; }

HRESULT STDMETHODCALLTYPE SkBaseIStream::Stat(STATSTG* pStatstg
                                            , DWORD grfStatFlag)
{ return E_NOTIMPL; }


/**
 * SkIStream
 */
SkIStream::SkIStream(SkStream* stream, bool unrefOnRelease)
    : SkBaseIStream()
    , fSkStream(stream)
    , fUnrefOnRelease(unrefOnRelease)
{ }

SkIStream::~SkIStream() {
    if (NULL != this->fSkStream && fUnrefOnRelease) {
        this->fSkStream->unref();
    }
}

HRESULT SkIStream::CreateFromSkStream(SkStream* stream
                                    , bool unrefOnRelease
                                    , IStream ** ppStream)
{
    *ppStream = new SkIStream(stream, unrefOnRelease);
    return S_OK;
}

// ISequentialStream Interface
HRESULT STDMETHODCALLTYPE SkIStream::Read(void* pv, ULONG cb, ULONG* pcbRead) {
    *pcbRead = this->fSkStream->read(pv, cb);
    return (*pcbRead == cb) ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE SkIStream::Write(void const* pv
                                         , ULONG cb
                                         , ULONG* pcbWritten)
{
    return STG_E_CANTSAVE;
}

// IStream Interface
HRESULT STDMETHODCALLTYPE SkIStream::Seek(LARGE_INTEGER liDistanceToMove
                                        , DWORD dwOrigin
                                        , ULARGE_INTEGER* lpNewFilePointer)
{
    if (lpNewFilePointer != NULL) {
        (*lpNewFilePointer).QuadPart = NULL;
    }
        
    HRESULT hr = S_OK;
    switch(dwOrigin) {
    case STREAM_SEEK_SET: {
        if (!this->fSkStream->rewind()) {
            hr = E_FAIL;
        } else {
            size_t skipped = this->fSkStream->skip(
                liDistanceToMove.QuadPart
            );
            if (skipped != liDistanceToMove.QuadPart) {
                hr = E_FAIL;
            }
        }
        break;
    }
    case STREAM_SEEK_CUR: {
        size_t skipped = this->fSkStream->skip(liDistanceToMove.QuadPart);
        if (skipped != liDistanceToMove.QuadPart) {
            hr = E_FAIL;
        }
        break;
    }
    case STREAM_SEEK_END: {
        if (!this->fSkStream->rewind()) {
            hr = E_FAIL;
        } else {
            size_t skipped = this->fSkStream->skip(
                this->fSkStream->getLength() + liDistanceToMove.QuadPart
            );
            if (skipped != liDistanceToMove.QuadPart) {
                hr = E_FAIL;
            }
        }
        break;
    }
    default:
        hr = STG_E_INVALIDFUNCTION;
        break;
    }
    
    return hr;
}

HRESULT STDMETHODCALLTYPE SkIStream::Stat(STATSTG* pStatstg
                                        , DWORD grfStatFlag)
{
    if (0 == grfStatFlag & STATFLAG_NONAME) {
        return STG_E_INVALIDFLAG;
    }
    pStatstg->pwcsName = NULL;
    pStatstg->cbSize.QuadPart = this->fSkStream->getLength();
    pStatstg->clsid = CLSID_NULL;
    pStatstg->type = STGTY_STREAM;
    pStatstg->grfMode = STGM_READ;
    return S_OK;
}


/**
 * SkIWStream
 */
SkWIStream::SkWIStream(SkWStream* stream)
    : SkBaseIStream()
    , fSkWStream(stream)
{ }

SkWIStream::~SkWIStream() {
    if (NULL != this->fSkWStream) {
        this->fSkWStream->flush();
    }
}

HRESULT SkWIStream::CreateFromSkWStream(SkWStream* stream
                                      , IStream ** ppStream)
{
    *ppStream = new SkWIStream(stream);
    return S_OK;
}

// ISequentialStream Interface
HRESULT STDMETHODCALLTYPE SkWIStream::Write(void const* pv
                                          , ULONG cb
                                          , ULONG* pcbWritten)
{
    HRESULT hr = S_OK;
    bool wrote = this->fSkWStream->write(pv, cb);
    if (wrote) {
        *pcbWritten = cb;
    } else {
        *pcbWritten = 0;
        hr = S_FALSE;
    }
    return hr;
}

// IStream Interface
HRESULT STDMETHODCALLTYPE SkWIStream::Stat(STATSTG* pStatstg
                                         , DWORD grfStatFlag)
{
    if (0 == grfStatFlag & STATFLAG_NONAME) {
        return STG_E_INVALIDFLAG;
    }
    pStatstg->pwcsName = NULL;
    pStatstg->cbSize.QuadPart = 0;
    pStatstg->clsid = CLSID_NULL;
    pStatstg->type = STGTY_STREAM;
    pStatstg->grfMode = STGM_WRITE;
    return S_OK;
}
