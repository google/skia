/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegEncoder_DEFINED
#define SkJpegEncoder_DEFINED

#include "SkPixmap.h"
#include "SkTypes.h"

class SkWStream;

class SkJpegEncoder : SkNoncopyable {
public:

    // TODO (skbug.com/1501):
    // Since jpegs are always opaque, this encoder ignores the alpha channel and treats the
    // pixels as opaque.
    // Another possible behavior is to blend the pixels onto opaque black.  We'll need to add
    // an option for this - and an SkTransferFunctionBehavior.

    struct Options {
        /**
         * |fQuality| must be in [0, 100] where 0 corresponds to the lowest quality.
         */
        int fQuality = 100;
    };

    /**
     *  Encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  Returns true on success.  Returns false on an invalid or unsupported |src|.
     */
    static bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options);

    /**
     *  Create a jpeg encoder that will encode the |src| pixels to the |dst| stream.
     *  |options| may be used to control the encoding behavior.
     *
     *  |dst| is unowned but must remain valid for the lifetime of the object.
     *
     *  This returns nullptr on an invalid or unsupported |src|.
     */
    static std::unique_ptr<SkJpegEncoder> Make(SkWStream* dst, const SkPixmap& src,
                                               const Options& options);

    /**
     *  Encode |numRows| rows of input.  If the caller requests more rows than are remaining
     *  in the src, this will encode all of the remaining rows.  |numRows| must be greater
     *  than zero.
     */
    bool encodeRows(int numRows);
};

#endif
