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


static sk_managedwstream_write_delegate        gWrite;
static sk_managedwstream_flush_delegate        gFlush;
static sk_managedwstream_bytesWritten_delegate gBytesWritten;
static sk_managedwstream_destroy_delegate      gWDestroy;

static inline SkManagedWStream* AsManagedWStream(sk_wstream_managedstream_t* cstream) {
    return reinterpret_cast<SkManagedWStream*>(cstream);
}
static inline sk_wstream_managedstream_t* ToManagedWStream(SkManagedWStream* stream) {
    return reinterpret_cast<sk_wstream_managedstream_t*>(stream);
}
static inline const sk_wstream_managedstream_t* ToManagedWStream(const SkManagedWStream* stream) {
    return reinterpret_cast<const sk_wstream_managedstream_t*>(stream);
}

bool dWrite(SkManagedWStream* managedStream, const void* buffer, size_t size)
{
    return gWrite(ToManagedWStream(managedStream), buffer, size);
}
void dFlush(SkManagedWStream* managedStream)
{
    gFlush(ToManagedWStream(managedStream));
}
size_t dBytesWritten(const SkManagedWStream* managedStream)
{
    return gBytesWritten(ToManagedWStream(managedStream));
}
void dWDestroy(size_t managedStream)
{
    gWDestroy(managedStream);
}

sk_wstream_managedstream_t* sk_managedwstream_new()
{
    return ToManagedWStream(new SkManagedWStream());
}

void sk_managedwstream_destroy(sk_wstream_managedstream_t* stream)
{
    delete AsManagedWStream(stream);
}

void sk_managedwstream_set_delegates(const sk_managedwstream_write_delegate pWrite,
                                     const sk_managedwstream_flush_delegate pFlush,
                                     const sk_managedwstream_bytesWritten_delegate pBytesWritten,
                                     const sk_managedwstream_destroy_delegate pDestroy)
{
    gWrite = pWrite;
    gFlush = pFlush;
    gBytesWritten = pBytesWritten;
    gWDestroy = pDestroy;

    SkManagedWStream::setDelegates(dWrite, dFlush, dBytesWritten, dWDestroy);
}


static sk_managedstream_read_delegate         gRead;
static sk_managedstream_peek_delegate         gPeek;
static sk_managedstream_isAtEnd_delegate      gIsAtEnd;
static sk_managedstream_hasPosition_delegate  gHasPosition;
static sk_managedstream_hasLength_delegate    gHasLength;
static sk_managedstream_rewind_delegate       gRewind;
static sk_managedstream_getPosition_delegate  gGetPosition;
static sk_managedstream_seek_delegate         gSeek;
static sk_managedstream_move_delegate         gMove;
static sk_managedstream_getLength_delegate    gGetLength;
static sk_managedstream_createNew_delegate    gCreateNew;
static sk_managedstream_destroy_delegate      gDestroy;


static inline SkManagedStream* AsManagedStream(sk_stream_managedstream_t* cstream) {
    return reinterpret_cast<SkManagedStream*>(cstream);
}
static inline sk_stream_managedstream_t* ToManagedStream(SkManagedStream* stream) {
    return reinterpret_cast<sk_stream_managedstream_t*>(stream);
}
static inline const sk_stream_managedstream_t* ToManagedStream(const SkManagedStream* stream) {
    return reinterpret_cast<const sk_stream_managedstream_t*>(stream);
}


size_t dRead(SkManagedStream* managedStream, void* buffer, size_t size)
{
    return gRead(ToManagedStream(managedStream), buffer, size);
}
size_t dPeek(SkManagedStream* managedStream, void* buffer, size_t size)
{
    return gPeek(ToManagedStream(managedStream), buffer, size);
}
bool dIsAtEnd(const SkManagedStream* managedStream)
{
    return gIsAtEnd(ToManagedStream(managedStream));
}
bool dHasPosition(const SkManagedStream* managedStream)
{
    return gHasPosition(ToManagedStream(managedStream));
}
bool dHasLength(const SkManagedStream* managedStream)
{
    return gHasLength(ToManagedStream(managedStream));
}
bool dRewind(SkManagedStream* managedStream)
{
    return gRewind(ToManagedStream(managedStream));
}
size_t dGetPosition(const SkManagedStream* managedStream)
{
    return gGetPosition(ToManagedStream(managedStream));
}
bool dSeek(SkManagedStream* managedStream, size_t position)
{
    return gSeek(ToManagedStream(managedStream), position);
}
bool dMove(SkManagedStream* managedStream, long offset)
{
    return gMove(ToManagedStream(managedStream), offset);
}
size_t dGetLength(const SkManagedStream* managedStream)
{
    return gGetLength(ToManagedStream(managedStream));
}
SkManagedStream* dCreateNew(const SkManagedStream* managedStream)
{
    return AsManagedStream(gCreateNew(ToManagedStream(managedStream)));
}
void dDestroy(size_t managedStream)
{
    gDestroy(managedStream);
}


sk_stream_managedstream_t* sk_managedstream_new ()
{
    return ToManagedStream (new SkManagedStream ());
}

void sk_managedstream_destroy (sk_stream_managedstream_t* stream)
{
    delete AsManagedStream (stream);
}

void sk_managedstream_set_delegates (const sk_managedstream_read_delegate pRead,
                                     const sk_managedstream_peek_delegate pPeek,
                                     const sk_managedstream_isAtEnd_delegate pIsAtEnd,
                                     const sk_managedstream_hasPosition_delegate pHasPosition,
                                     const sk_managedstream_hasLength_delegate pHasLength,
                                     const sk_managedstream_rewind_delegate pRewind,
                                     const sk_managedstream_getPosition_delegate pGetPosition,
                                     const sk_managedstream_seek_delegate pSeek,
                                     const sk_managedstream_move_delegate pMove,
                                     const sk_managedstream_getLength_delegate pGetLength,
                                     const sk_managedstream_createNew_delegate pCreateNew,
                                     const sk_managedstream_destroy_delegate pDestroy)
{
    gRead = pRead;
    gPeek = pPeek;
    gIsAtEnd = pIsAtEnd;
    gHasPosition = pHasPosition;
    gHasLength = pHasLength;
    gRewind = pRewind;
    gGetPosition = pGetPosition;
    gSeek = pSeek;
    gMove = pMove;
    gGetLength = pGetLength;
    gCreateNew = pCreateNew;
    gDestroy = pDestroy;

    SkManagedStream::setDelegates(dRead, dPeek, dIsAtEnd, dHasPosition, dHasLength, dRewind, dGetPosition, dSeek, dMove, dGetLength, dCreateNew, dDestroy);
}

