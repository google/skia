
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFlate_DEFINED
#define SkFlate_DEFINED

#include "SkTypes.h"

#include "SkStream.h"
class SkData;

/** \class SkFlate
    A class to provide access to the flate compression algorithm.
*/
class SkFlate {
public:
    /**
     *  Use the flate compression algorithm to compress the data in src,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(SkStream* src, SkWStream* dst);

    /**
     *  Use the flate compression algorithm to compress the data in src,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(const void* src, size_t len, SkWStream* dst);

    /**
     *  Use the flate compression algorithm to compress the data,
     *  putting the result into dst.  Returns false if an error occurs.
     */
    static bool Deflate(const SkData*, SkWStream* dst);

    /** Use the flate compression algorithm to decompress the data in src,
        putting the result into dst.  Returns false if an error occurs.
     */
    static bool Inflate(SkStream* src, SkWStream* dst);
};

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
    bool write(const void*, size_t) override;
    size_t bytesWritten() const override;

private:
    struct Impl;
    SkAutoTDelete<Impl> fImpl;
};

#endif  // SkFlate_DEFINED
