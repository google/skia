/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamBuffer_DEFINED
#define SkStreamBuffer_DEFINED

#include "SkStream.h"
#include "SkTypes.h"

/**
 *  Helper class for reading from a stream that may not have all its data
 *  available yet.
 *
 *  Used by GIFImageReader, and currently set up for that use case.
 *
 *  Buffers up to 256 * 3 bytes (256 colors, with 3 bytes each) to support GIF.
 *  FIXME (scroggo): Make this more general purpose?
 */
class SkStreamBuffer : SkNoncopyable {
public:
    // Takes ownership of the SkStream.
    SkStreamBuffer(SkStream*);

    /**
     *  Return a pointer the buffered data.
     *
     *  The number of bytes buffered is the sum of values returned by calls to
     *  buffer() since the last call to flush().
     */
    const char* get() const { SkASSERT(fBytesBuffered >= 1); return fBuffer; }

    /**
     *  Bytes in the buffer.
     *
     *  Sum of the values returned by calls to buffer() since the last call to
     *  flush().
     */
    size_t bytesBuffered() const { return fBytesBuffered; }

    /**
     *  Buffer from the stream into our buffer.
     *
     *  Returns the number of bytes successfully buffered.
     */
    size_t buffer(size_t bytes);

    /**
     *  Flush the buffer.
     *
     *  After this call, no bytes are buffered.
     */
    void flush() {
        fBytesBuffered = 0;
    }

private:
    static constexpr size_t kMaxSize = 256 * 3;

    std::unique_ptr<SkStream>   fStream;
    char                        fBuffer[kMaxSize];
    size_t                      fBytesBuffered;
};
#endif // SkStreamBuffer_DEFINED

