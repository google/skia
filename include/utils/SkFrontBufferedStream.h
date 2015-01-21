/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

class SkStream;
class SkStreamRewindable;

/**
 *  Specialized stream that buffers the first X bytes of a stream,
 *  where X is passed in by the user. Note that unlike some buffered
 *  stream APIs, once more bytes than can fit in the buffer are read,
 *  no more buffering is done. This stream is designed for a use case
 *  where the caller knows that rewind will only be called from within
 *  X bytes (inclusive), and the wrapped stream is not necessarily
 *  able to rewind at all.
 */
class SkFrontBufferedStream {
public:
    /**
     *  Creates a new stream that wraps and buffers an SkStream.
     *  @param stream SkStream to buffer. If stream is NULL, NULL is
     *      returned. When this call succeeds (i.e. returns non NULL),
     *      SkFrontBufferedStream is expected to be the only owner of
     *      stream, so it should no be longer used directly.
     *      SkFrontBufferedStream will delete stream upon deletion.
     *  @param minBufferSize Minimum size of buffer required.
     *  @return An SkStream that can buffer at least minBufferSize, or
     *      NULL on failure. The caller is required to delete when finished with
     *      this object.
     */
    static SkStreamRewindable* Create(SkStream* stream, size_t minBufferSize);
};
