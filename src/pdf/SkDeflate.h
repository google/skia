/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFlate_DEFINED
#define SkFlate_DEFINED

#include "include/core/SkStream.h"
#include <cstddef>

#include <memory>

/**
  * Wrap a stream in this class to compress the information written to
  * this stream using the Deflate algorithm.
  *
  * See http://en.wikipedia.org/wiki/DEFLATE
  */
class SkDeflateWStream final : public SkWStream {
public:
    /** Does not take ownership of the stream.

        @param compressionLevel 1 is best speed; 9 is best compression.
        The default, -1, is to use zlib's Z_DEFAULT_COMPRESSION level.
        0 would be no compression, but due to broken zlibs, users should handle that themselves.

        @param gzip iff true, output a gzip file. "The gzip format is
        a wrapper, documented in RFC 1952, around a deflate stream."
        gzip adds a header with a magic number to the beginning of the
        stream, allowing a client to identify a gzip file.
     */
    SkDeflateWStream(SkWStream*,
                     int compressionLevel,
                     bool gzip = false);

    /** The destructor calls finalize(). */
    ~SkDeflateWStream() override;

    /** Write the end of the compressed stream.  All subsequent calls to
        write() will fail. Subsequent calls to finalize() do nothing. */
    void finalize();

    // The SkWStream interface:
    bool write(const void*, size_t) override;
    size_t bytesWritten() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

#endif  // SkFlate_DEFINED
