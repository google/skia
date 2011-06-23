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

#ifndef SkIStream_DEFINED
#define SkIStream_DEFINED

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ole2.h>

class SkStream;
class SkWStream;

/**
 * A bare IStream implementation which properly reference counts
 * but returns E_NOTIMPL for all ISequentialStream and IStream methods.
 */
class SkBaseIStream : public IStream {
private:
    LONG _refcount;

protected:
    explicit SkBaseIStream();
    virtual ~SkBaseIStream();

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid
                                                   , void ** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    // ISequentialStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead);

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv
                                          , ULONG cb
                                          , ULONG* pcbWritten);

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER);
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*
                                           , ULARGE_INTEGER
                                           , ULARGE_INTEGER*
                                           , ULARGE_INTEGER*);
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void);
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER
                                               , ULARGE_INTEGER
                                               , DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER
                                                 , ULARGE_INTEGER
                                                 , DWORD);
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **);

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove
                                         , DWORD dwOrigin
                                         , ULARGE_INTEGER* lpNewFilePointer);

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg
                                         , DWORD grfStatFlag);
};

/**
 * A minimal read-only IStream implementation which wraps an SkIStream.
 */
class SkIStream : public SkBaseIStream {
private:
    SkStream *fSkStream;
    bool fUnrefOnRelease;

    SkIStream(SkStream* stream, bool unrefOnRelease);
    virtual ~SkIStream();

public:
    HRESULT static CreateFromSkStream(SkStream* stream
                                    , bool unrefOnRelease
                                    , IStream ** ppStream);

    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead);

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv
                                          , ULONG cb
                                          , ULONG* pcbWritten);

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove
                                         , DWORD dwOrigin
                                         , ULARGE_INTEGER* lpNewFilePointer);

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg
                                         , DWORD grfStatFlag);
};

/**
 * A minimal write-only IStream implementation which wraps an SkWIStream.
 */
class SkWIStream : public SkBaseIStream {
private:
    SkWStream *fSkWStream;

    SkWIStream(SkWStream* stream);
    virtual ~SkWIStream();

public:
    HRESULT static CreateFromSkWStream(SkWStream* stream, IStream ** ppStream);

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv
                                          , ULONG cb
                                          , ULONG* pcbWritten);

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg
                                         , DWORD grfStatFlag);
};

#endif
