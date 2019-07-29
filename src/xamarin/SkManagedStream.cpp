/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkManagedStream.h"

// READ-ONLY MANAGED STREAM

SkManagedStream::Procs SkManagedStream::fProcs;

void SkManagedStream::setProcs(SkManagedStream::Procs procs) {
    fProcs = procs;
}

SkManagedStream::SkManagedStream(void* context) {
    fContext = context;
}
SkManagedStream::~SkManagedStream() {
    if (!fProcs.fDestroy) return;
    fProcs.fDestroy(this, fContext);
}

size_t SkManagedStream::read(void* buffer, size_t size) {
    if (!fProcs.fRead) return 0;
    return fProcs.fRead(this, fContext, buffer, size);
}
size_t SkManagedStream::peek(void *buffer, size_t size) const {
    if (!fProcs.fPeek) return 0;
    return fProcs.fPeek(this, fContext, buffer, size);
}
bool SkManagedStream::isAtEnd() const {
    if (!fProcs.fIsAtEnd) return false;
    return fProcs.fIsAtEnd(this, fContext);
}
bool SkManagedStream::hasPosition() const {
    if (!fProcs.fHasPosition) return false;
    return fProcs.fHasPosition(this, fContext);
}
bool SkManagedStream::hasLength() const {
    if (!fProcs.fHasLength) return false;
    return fProcs.fHasLength(this, fContext);
}
bool SkManagedStream::rewind() {
    if (!fProcs.fRewind) return false;
    return fProcs.fRewind(this, fContext);
}
size_t SkManagedStream::getPosition() const {
    if (!fProcs.fGetPosition) return 0;
    return fProcs.fGetPosition(this, fContext);
}
bool SkManagedStream::seek(size_t position) {
    if (!fProcs.fSeek) return false;
    return fProcs.fSeek(this, fContext, position);
}
bool SkManagedStream::move(long offset) {
    if (!fProcs.fMove) return false;
    return fProcs.fMove(this, fContext, offset);
}
size_t SkManagedStream::getLength() const {
    if (!fProcs.fGetLength) return 0;
    return fProcs.fGetLength(this, fContext);
}
SkStreamAsset* SkManagedStream::onDuplicate() const {
    if (!fProcs.fDuplicate) return nullptr;
    return fProcs.fDuplicate(this, fContext);
}
SkStreamAsset* SkManagedStream::onFork() const {
    if (!fProcs.fFork) return nullptr;
    return fProcs.fFork(this, fContext);
}


// WRITEABLE MANAGED STREAM

SkManagedWStream::Procs SkManagedWStream::fProcs;

void SkManagedWStream::setProcs(SkManagedWStream::Procs procs) {
    fProcs = procs;
}

SkManagedWStream::SkManagedWStream(void* context) {
    fContext = context;
}
SkManagedWStream::~SkManagedWStream() {
    if (!fProcs.fDestroy) return;
    fProcs.fDestroy(this, fContext);
}

bool SkManagedWStream::write(const void* buffer, size_t size) {
    if (!fProcs.fWrite) return false;
    return fProcs.fWrite(this, fContext, buffer, size);
}
void SkManagedWStream::flush() {
    if (!fProcs.fFlush) return;
    fProcs.fFlush(this, fContext);
}
size_t SkManagedWStream::bytesWritten() const {
    if (!fProcs.fBytesWritten) return 0;
    return fProcs.fBytesWritten(this, fContext);
}
