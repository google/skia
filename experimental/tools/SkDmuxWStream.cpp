/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDmuxWStream.h"

SkDmuxWStream::SkDmuxWStream(SkWStream* const streamArray[], size_t count)
    : fWStreams(streamArray, static_cast<int>(count)), fBytesWritten(0) {}

SkDmuxWStream::~SkDmuxWStream() {
    for (int i = 0; i < fWStreams.count(); ++i) {
        fWStreams[i]->flush();
    }
}

bool SkDmuxWStream::write(const void* buffer, size_t size) {
    for (int i = 0; i < fWStreams.count(); ++i) {
        if (!fWStreams[i]->write(buffer, size)) {
            return false;
        }
    }
    fBytesWritten += size;
    return true;
}

void SkDmuxWStream::newline() {
    for (int i = 0; i < fWStreams.count(); ++i) {
        fWStreams[i]->newline();
    }
    fBytesWritten += 1;  // This may be a lie.
}

void SkDmuxWStream::flush() {
    for (int i = 0; i < fWStreams.count(); ++i) {
        fWStreams[i]->flush();
    }
}

size_t SkDmuxWStream::bytesWritten() const { return fBytesWritten; }
