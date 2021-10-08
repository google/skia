/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStreamBuffer_DEFINED
#define SkStreamBuffer_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"

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
    SkStreamBuffer(std::unique_ptr<SkStream>);
    ~SkStreamBuffer();

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
     *  from the stream. In addition, markPosition() can be called to mark this
     *  position and enable calling getAtPosition() later to retrieve |bytes|
     *  bytes.
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
        fPosition += fBytesBuffered;
        fBytesBuffered = 0;
    }

    /**
     *  Mark the current position in the stream to return to it later.
     *
     *  This is the position of the start of the buffer. After this call, a
     *  a client can call getDataAtPosition to retrieve all the bytes currently
     *  buffered.
     *
     *  @return size_t Position which can be passed to getDataAtPosition later
     *      to retrieve the data currently buffered.
     */
    size_t markPosition();

    /**
     *  Retrieve data at position, as previously marked by markPosition().
     *
     *  @param position Position to retrieve data, as marked by markPosition().
     *  @param length   Amount of data required at position.
     *  @return SkData The data at position.
     */
    sk_sp<SkData> getDataAtPosition(size_t position, size_t length);

private:
    inline static constexpr size_t kMaxSize = 256 * 3;

    std::unique_ptr<SkStream>   fStream;
    size_t                      fPosition;
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
    // Only used if !fHasLengthAndPosition. In that case, markPosition will
    // copy into an SkData, stored here.
    SkTHashMap<size_t, SkData*> fMarkedData;
};
#endif // SkStreamBuffer_DEFINED

