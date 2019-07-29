/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkManagedStream_h
#define SkManagedStream_h

#include "SkTypes.h"
#include "SkStream.h"

class SkManagedWStream;
class SkManagedStream;

// READ-ONLY MANAGED STREAM

class SkManagedStream : public SkStreamAsset {
public:
    SkManagedStream(void* context);

    virtual ~SkManagedStream();

    size_t read(void* buffer, size_t size) override;
    bool isAtEnd() const override;
    bool hasPosition() const override;
    bool hasLength() const override;

    size_t peek(void* buffer, size_t size) const override;

    bool rewind() override;

    size_t getPosition() const override;
    bool seek(size_t position) override;
    bool move(long offset) override;

    size_t getLength() const override;

public:
    typedef size_t            (*ReadProc)        (      SkManagedStream* s, void* context, void* buffer, size_t size);
    typedef size_t            (*PeekProc)        (const SkManagedStream* s, void* context, void* buffer, size_t size);
    typedef bool              (*IsAtEndProc)     (const SkManagedStream* s, void* context);
    typedef bool              (*HasPositionProc) (const SkManagedStream* s, void* context);
    typedef bool              (*HasLengthProc)   (const SkManagedStream* s, void* context);
    typedef bool              (*RewindProc)      (      SkManagedStream* s, void* context);
    typedef size_t            (*GetPositionProc) (const SkManagedStream* s, void* context);
    typedef bool              (*SeekProc)        (      SkManagedStream* s, void* context, size_t position);
    typedef bool              (*MoveProc)        (      SkManagedStream* s, void* context, long offset);
    typedef size_t            (*GetLengthProc)   (const SkManagedStream* s, void* context);
    typedef SkManagedStream*  (*DuplicateProc)   (const SkManagedStream* s, void* context);
    typedef SkManagedStream*  (*ForkProc)        (const SkManagedStream* s, void* context);
    typedef void              (*DestroyProc)     (      SkManagedStream* s, void* context);

    struct Procs {
        ReadProc fRead = nullptr;
        PeekProc fPeek = nullptr;
        IsAtEndProc fIsAtEnd = nullptr;
        HasPositionProc fHasPosition = nullptr;
        HasLengthProc fHasLength = nullptr;
        RewindProc fRewind = nullptr;
        GetPositionProc fGetPosition = nullptr;
        SeekProc fSeek = nullptr;
        MoveProc fMove = nullptr;
        GetLengthProc fGetLength = nullptr;
        DuplicateProc fDuplicate = nullptr;
        ForkProc fFork = nullptr;
        DestroyProc fDestroy = nullptr;
    };

    static void setProcs(SkManagedStream::Procs procs);

private:
    SkStreamAsset* onDuplicate() const override;
    SkStreamAsset* onFork() const override;

private:
    void* fContext;
    static Procs fProcs;

    typedef SkStreamAsset INHERITED;
};


// WRITEABLE MANAGED STREAM

class SkManagedWStream : public SkWStream {
public:
    SkManagedWStream(void* context);

    virtual ~SkManagedWStream();

    bool write(const void* buffer, size_t size) override;
    void flush() override;
    size_t bytesWritten() const override;

public:
    typedef bool     (*WriteProc)        (      SkManagedWStream* s, void* context, const void* buffer, size_t size);
    typedef void     (*FlushProc)        (      SkManagedWStream* s, void* context);
    typedef size_t   (*BytesWrittenProc) (const SkManagedWStream* s, void* context);
    typedef void     (*DestroyProc)      (      SkManagedWStream* s, void* context);

    struct Procs {
        WriteProc fWrite = nullptr;
        FlushProc fFlush = nullptr;
        BytesWrittenProc fBytesWritten = nullptr;
        DestroyProc fDestroy = nullptr;
    };

    static void setProcs(SkManagedWStream::Procs procs);

private:
    void* fContext;
    static Procs fProcs;

    typedef SkWStream INHERITED;
};

#endif
