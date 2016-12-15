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
     *  The number of bytes buffered is the number passed to buffer()
     *  after the last call to flush().
     */
    const char* get() const;

    /**
     *  Buffer from the stream into our buffer.
     *
     *  If this call returns true, get() can be used to access |bytes| bytes
     *  from the stream, or if rewindable(), readFromPosition can be used to
     *  read these bytes now or later.
     *
     *  @param bytes Total number of bytes desired.
     *
     *  @return Whether all bytes were successfully buffered.
     */
    bool buffer(size_t bytes);

    /**
     *  Flush the buffer.
     *
     *  After this call, no bytes are buffered.
     */
    void flush() {
        if (fHasLengthAndPosition) {
            if (fTrulyBuffered < fBytesBuffered) {
                fStream->move(fBytesBuffered - fTrulyBuffered);
            }
            fTrulyBuffered = 0;
        }
        fBytesBuffered = 0;
    }

    /**
     *  Whether the stream can be rewound.
     *
     *  True if underlying stream hasLength() and hasPosition().
     *
     *  If true, the client can call readFromPosition instead of copying the
     *  bytes returned by get().
     */
    bool rewindable() const { return fHasLengthAndPosition; }

    /**
     *  Copy data into dst. Only works if rewindable().
     *
     *  After this call, the state will be unchanged unless there was an
     *  error.
     *
     *  @param dst Destination to copy into.
     *  @param offset Offset in underlying stream to copy.
     *  @param length Number of bytes to copy.
     *  @return Whether the copy succeeded.
     */
    bool readFromPosition(void* dst, size_t offset, size_t length);

    /**
     *  Return the position of the stream.
     *
     *  Does not count any bytes currently buffered.
     *
     *  Only meaningful if rewindable() returns true.
     */
    size_t getPosition() const;

private:
    static constexpr size_t kMaxSize = 256 * 3;

    std::unique_ptr<SkStream>   fStream;
    char                        fBuffer[kMaxSize];
    size_t                      fBytesBuffered;
    // If the stream has a length and position, we can make two optimizations:
    // - We can skip buffering
    // - During parsing, we can store the position and size of data that is
    //   needed later during decoding.
    const bool                  fHasLengthAndPosition;
    // When fHasLengthAndPosition is true, we do not need to actually buffer
    // inside buffer(). We'll buffer inside get(). This keeps track of how many
    // bytes we've buffered inside get(), for the (non-existent) case of:
    //  buffer(n)
    //  get()
    //  buffer(n + u)
    //  get()
    // The second call to get() needs to only truly buffer the part that was
    // not already buffered.
    mutable size_t              fTrulyBuffered;
};
#endif // SkStreamBuffer_DEFINED

