/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedStream.h"


// read stream
static read_delegate fRead = nullptr;
static peek_delegate fPeek = nullptr;
static isAtEnd_delegate fIsAtEnd = nullptr;
static hasPosition_delegate fHasPosition = nullptr;
static hasLength_delegate fHasLength = nullptr;
static rewind_delegate fRewind = nullptr;
static getPosition_delegate fGetPosition = nullptr;
static seek_delegate fSeek = nullptr;
static move_delegate fMove = nullptr;
static getLength_delegate fGetLength = nullptr;
static createNew_delegate fCreateNew = nullptr;
static destroy_delegate fDestroy = nullptr;

// write stream
static write_delegate fWrite = nullptr;
static flush_delegate fFlush = nullptr;
static bytesWritten_delegate fBytesWritten = nullptr;
static wdestroy_delegate fWDestroy = nullptr;


// the read stream

SkManagedStream::SkManagedStream() {
    this->address = (size_t)this;
}

SkManagedStream::~SkManagedStream() {
    ::fDestroy(address);
}

void SkManagedStream::setDelegates(const read_delegate pRead,
                                   const peek_delegate pPeek,
                                   const isAtEnd_delegate pIsAtEnd,
                                   const hasPosition_delegate pHasPosition,
                                   const hasLength_delegate pHasLength,
                                   const rewind_delegate pRewind,
                                   const getPosition_delegate pGetPosition,
                                   const seek_delegate pSeek,
                                   const move_delegate pMove,
                                   const getLength_delegate pGetLength,
                                   const createNew_delegate pCreateNew,
                                   const destroy_delegate pDestroy)
{
    ::fRead = (pRead);
    ::fPeek = (pPeek);
    ::fIsAtEnd = (pIsAtEnd);
    ::fHasPosition = (pHasPosition);
    ::fHasLength = (pHasLength);
    ::fRewind = (pRewind);
    ::fGetPosition = (pGetPosition);
    ::fSeek = (pSeek);
    ::fMove = (pMove);
    ::fGetLength = (pGetLength);
    ::fCreateNew = (pCreateNew);
    ::fDestroy = (pDestroy);
}


size_t SkManagedStream::read(void* buffer, size_t size) {
    return ::fRead(this, buffer, size);
}

size_t SkManagedStream::peek(void *buffer, size_t size) const {
    SkManagedStream* nonConstThis = const_cast<SkManagedStream*>(this);
    return ::fPeek(nonConstThis, buffer, size);
}

bool SkManagedStream::isAtEnd() const {
    return ::fIsAtEnd(this);
}

bool SkManagedStream::hasPosition() const {
    return ::fHasPosition(this);
}

bool SkManagedStream::hasLength() const {
    return ::fHasLength(this);
}

bool SkManagedStream::rewind() {
    return ::fRewind(this);
}

size_t SkManagedStream::getPosition() const {
    return ::fGetPosition(this);
}

bool SkManagedStream::seek(size_t position) {
    return ::fSeek(this, position);
}

bool SkManagedStream::move(long offset) {
    return ::fMove(this, offset);
}

size_t SkManagedStream::getLength() const {
    return ::fGetLength(this);
}

SkManagedStream* SkManagedStream::duplicate() const {
    return ::fCreateNew(this);
}

SkManagedStream* SkManagedStream::fork() const {
    std::unique_ptr<SkManagedStream> that(::fCreateNew(this));
    that->seek(getPosition());
    return that.release();
}


// the write stream

SkManagedWStream::SkManagedWStream() {
    this->address = (size_t)this;
}

SkManagedWStream::~SkManagedWStream() {
    ::fWDestroy(address);
}

void SkManagedWStream::setDelegates(const write_delegate pWrite,
                                    const flush_delegate pFlush,
                                    const bytesWritten_delegate pBytesWritten,
                                    const wdestroy_delegate pDestroy)
{
    ::fWrite = (pWrite);
    ::fFlush = (pFlush);
    ::fBytesWritten = (pBytesWritten);
    ::fWDestroy = (pDestroy);
}

bool SkManagedWStream::write(const void* buffer, size_t size) {
    return ::fWrite(this, buffer, size);
}

void SkManagedWStream::flush() {
    ::fFlush(this);
}

size_t SkManagedWStream::bytesWritten() const {
    return ::fBytesWritten(this);
}
