/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDataUtils_DEFINED
#define SkDataUtils_DEFINED

#include "SkData.h"
#include "SkStream.h"

/**
 * Static class that performs various operations on SkData objects.
 *
 * EPOGER: Before committing, add unittests for these methods.
 *
 * TODO(epoger): Move these methods into SkStream.[cpp|h], as attempted in
 * https://codereview.appspot.com/7300071 ?
 */
class SkDataUtils {
public:
    /**
     * Read as many bytes as possible (up to maxBytes) from the stream into
     * an SkData object.
     *
     * If the returned SkData contains fewer than maxBytes, then EOF has been
     * reached and no more data would be available from subsequent calls.
     * (If EOF has already been reached, then this call will return an empty
     * SkData object immediately.)
     *
     * If there are fewer than maxBytes bytes available to read from the
     * stream, but the stream has not been closed yet, this call will block
     * until there are enough bytes to read or the stream has been closed.
     *
     * It is up to the caller to call unref() on the returned SkData object
     * once the data is no longer needed, so that the underlying buffer will
     * be freed.  For example:
     *
     * {
     *   size_t maxBytes = 256;
     *   SkAutoDataUnref dataRef(readIntoSkData(stream, maxBytes));
     *   if (NULL != dataRef.get()) {
     *     size_t bytesActuallyRead = dataRef.get()->size();
     *     // use the data...
     *   }
     * }
     * // underlying buffer has been freed, thanks to auto unref
     */
    static SkData* ReadIntoSkData(SkStream &stream, size_t maxBytes);

    /**
     * Wrapper around ReadIntoSkData for files: reads the entire file into
     * an SkData object.
     */
    static SkData* ReadFileIntoSkData(SkFILEStream &stream) {
        return ReadIntoSkData(stream, stream.getLength());
    }

};

#endif
