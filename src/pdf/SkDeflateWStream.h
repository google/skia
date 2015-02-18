/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeflateWStream_DEFINED
#define SkDeflateWStream_DEFINED

#include "SkStream.h"
// https://skia.org/dev/contrib/style#no-define-before-sktypes

#ifndef SK_NO_FLATE  // Clients of this class should be guarded.

/**
 * Wrap a stream in this class to compress the information written to
 * this stream using the Deflate algorithm.  Uses Zlib's
 * Z_DEFAULT_COMPRESSION level.
 *
 * See http://en.wikipedia.org/wiki/DEFLATE
 */
class SkDeflateWStream : public SkWStream {
public:
    /** Does not take ownership of the stream. */
    SkDeflateWStream(SkWStream*);

    /** The destructor calls finalize(). */
    ~SkDeflateWStream();

    /** Write the end of the compressed stream.  All subsequent calls to
        write() will fail. Subsequent calls to finalize() do nothing. */
    void finalize();

    // The SkWStream interface:
    bool write(const void*, size_t) SK_OVERRIDE;
    size_t bytesWritten() const SK_OVERRIDE;

private:
    struct Impl;
    SkAutoTDelete<Impl> fImpl;
};
#endif  // SK_NO_FLATE

#endif  // SkDeflateWStream_DEFINED
