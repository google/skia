/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedStream.h"

#include "sk_managedstream.h"
#include "sk_types_priv.h"

// WRITEABLE MANAGED STREAM

static inline SkManagedWStream* AsManagedWStream(sk_wstream_managedstream_t* stream) {
    return reinterpret_cast<SkManagedWStream*>(stream);
}
static inline sk_wstream_managedstream_t* ToManagedWStream(SkManagedWStream* stream) {
    return reinterpret_cast<sk_wstream_managedstream_t*>(stream);
}
static inline const sk_wstream_managedstream_t* ToManagedWStream(const SkManagedWStream* stream) {
    return reinterpret_cast<const sk_wstream_managedstream_t*>(stream);
}

static sk_managedwstream_procs_t gWProcs;

bool dWrite(SkManagedWStream* stream, void* context, const void* buffer, size_t size) {
    if (!gWProcs.fWrite) return false;
    return gWProcs.fWrite(ToManagedWStream(stream), context, buffer, size);
}
void dFlush(SkManagedWStream* stream, void* context) {
    if (!gWProcs.fFlush) return;
    gWProcs.fFlush(ToManagedWStream(stream), context);
}
size_t dBytesWritten(const SkManagedWStream* stream, void* context) {
    if (!gWProcs.fBytesWritten) return 0;
    return gWProcs.fBytesWritten(ToManagedWStream(stream), context);
}
void dWDestroy(SkManagedWStream* stream, void* context) {
    if (!gWProcs.fDestroy) return;
    gWProcs.fDestroy(ToManagedWStream(stream), context);
}

sk_wstream_managedstream_t* sk_managedwstream_new(void* context) {
    return ToManagedWStream(new SkManagedWStream(context));
}
void sk_managedwstream_destroy(sk_wstream_managedstream_t* stream) {
    if (stream)
        delete AsManagedWStream(stream);
}
void sk_managedwstream_set_procs(sk_managedwstream_procs_t procs) {
    gWProcs = procs;

    SkManagedWStream::Procs p;
    p.fWrite = dWrite;
    p.fFlush = dFlush;
    p.fBytesWritten = dBytesWritten;
    p.fDestroy = dWDestroy;

    SkManagedWStream::setProcs(p);
}


// READ-ONLY MANAGED STREAM

static inline SkManagedStream* AsManagedStream(sk_stream_managedstream_t* s) {
    return reinterpret_cast<SkManagedStream*>(s);
}
static inline sk_stream_managedstream_t* ToManagedStream(SkManagedStream* s) {
    return reinterpret_cast<sk_stream_managedstream_t*>(s);
}
static inline const sk_stream_managedstream_t* ToManagedStream(const SkManagedStream* s) {
    return reinterpret_cast<const sk_stream_managedstream_t*>(s);
}

static sk_managedstream_procs_t gProcs;

size_t dRead(SkManagedStream* stream, void* context, void* buffer, size_t size) {
    if (!gProcs.fRead) return 0;
    return gProcs.fRead(ToManagedStream(stream), context, buffer, size);
}
size_t dPeek(const SkManagedStream* stream, void* context, void* buffer, size_t size) {
    if (!gProcs.fPeek) return 0;
    return gProcs.fPeek(ToManagedStream(stream), context, buffer, size);
}
bool dIsAtEnd(const SkManagedStream* stream, void* context) {
    if (!gProcs.fIsAtEnd) return false;
    return gProcs.fIsAtEnd(ToManagedStream(stream), context);
}
bool dHasPosition(const SkManagedStream* stream, void* context) {
    if (!gProcs.fIsAtEnd) return false;
    return gProcs.fHasPosition(ToManagedStream(stream), context);
}
bool dHasLength(const SkManagedStream* stream, void* context) {
    if (!gProcs.fHasLength) return false;
    return gProcs.fHasLength(ToManagedStream(stream), context);
}
bool dRewind(SkManagedStream* stream, void* context) {
    if (!gProcs.fRewind) return false;
    return gProcs.fRewind(ToManagedStream(stream), context);
}
size_t dGetPosition(const SkManagedStream* stream, void* context) {
    if (!gProcs.fGetPosition) return 0;
    return gProcs.fGetPosition(ToManagedStream(stream), context);
}
bool dSeek(SkManagedStream* stream, void* context, size_t position) {
    if (!gProcs.fSeek) return false;
    return gProcs.fSeek(ToManagedStream(stream), context, position);
}
bool dMove(SkManagedStream* stream, void* context, long offset) {
    if (!gProcs.fMove) return false;
    return gProcs.fMove(ToManagedStream(stream), context, offset);
}
size_t dGetLength(const SkManagedStream* stream, void* context) {
    if (!gProcs.fGetLength) return 0;
    return gProcs.fGetLength(ToManagedStream(stream), context);
}
SkManagedStream* dDuplicate(const SkManagedStream* stream, void* context) {
    if (!gProcs.fDuplicate) return nullptr;
    return AsManagedStream(gProcs.fDuplicate(ToManagedStream(stream), context));
}
SkManagedStream* dFork(const SkManagedStream* stream, void* context) {
    if (!gProcs.fFork) return nullptr;
    return AsManagedStream(gProcs.fFork(ToManagedStream(stream), context));
}
void dDestroy(SkManagedStream* stream, void* context) {
    if (!gProcs.fDestroy) return;
    gProcs.fDestroy(ToManagedStream(stream), context);
}

sk_stream_managedstream_t* sk_managedstream_new(void* context) {
    return ToManagedStream(new SkManagedStream(context));
}
void sk_managedstream_destroy(sk_stream_managedstream_t* stream) {
    if (stream)
        delete AsManagedStream(stream);
}
void sk_managedstream_set_procs(const sk_managedstream_procs_t procs) {
    gProcs = procs;

    SkManagedStream::Procs p;
    p.fRead = dRead;
    p.fPeek = dPeek;
    p.fIsAtEnd = dIsAtEnd;
    p.fHasPosition = dHasPosition;
    p.fHasLength = dHasLength;
    p.fRewind = dRewind;
    p.fGetPosition = dGetPosition;
    p.fSeek = dSeek;
    p.fMove = dMove;
    p.fGetLength = dGetLength;
    p.fDuplicate = dDuplicate;
    p.fFork = dFork;
    p.fDestroy = dDestroy;

    SkManagedStream::setProcs(p);
}
