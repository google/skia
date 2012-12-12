
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapTransformer_DEFINED
#define SkBitmapTransformer_DEFINED

#include "SkBitmap.h"

/**
 * Class that can copy pixel data out of an SkBitmap, transforming it
 * into the appropriate PixelFormat.
 *
 * As noted in https://codereview.appspot.com/6849119/#msg6 and
 * https://codereview.appspot.com/6900047 , at some point we might want
 * to make this more general purpose:
 * - support more PixelFormats
 * - use existing SkCanvas::Config8888 enum instead of new PixelFormat enum
 * - add method to copy pixel data for a single row, instead of the whole bitmap
 * - add methods to copy pixel data INTO an SkBitmap
 *
 * That would allow us to replace SkCopyConfig8888ToBitmap() in
 * src/core/SkConfig8888.h , as well as the transformations used by
 * src/images/SkImageDecoder_libpng.cpp , with this common code.
 *
 * But for now, we want something more narrowly targeted, just
 * supplying what is needed by SkBitmapChecksummer.
 */
class SkBitmapTransformer {
public:
    enum PixelFormat {
        // 32 bits per pixel, ARGB byte order, with the alpha-channel
        // value premultiplied into the R/G/B channel values.
        kARGB_8888_Premul_PixelFormat,

        // marks the end of the list
        kLast_PixelFormat = kARGB_8888_Premul_PixelFormat,
    };

    /**
     * Creates an SkBitmapTransformer instance that can transform between
     * the given bitmap and a pixel buffer with given pixelFormat.
     *
     * Call IsValid() before using, to confirm that this particular
     * bitmap/pixelFormat combination is supported!
     */
    SkBitmapTransformer(const SkBitmap& bitmap, PixelFormat pixelFormat) :
        fBitmap(bitmap), fPixelFormat(pixelFormat) {}

    /**
     * Returns true iff we can convert between fBitmap and fPixelFormat.
     * If this returns false, the return values of any other methods will
     * be meaningless!
     *
     * @param logReason whether to log the reason why this combination
     *                  is unsupported (only applies in debug mode)
     */
    bool isValid(bool logReason=false) const;

    /**
     * Returns the number of bytes needed to store a single row of the
     * bitmap's pixels if converted to pixelFormat.
     */
    size_t bytesNeededPerRow() const {
        // This is hard-coded for the single supported PixelFormat.
        return fBitmap.width() * 4;
    }

    /**
     * Returns the number of bytes needed to store the entire bitmap
     * if converted to pixelFormat, ASSUMING that it is written
     * out as a single contiguous blob of pixels (no leftover bytes
     * at the end of each row).
     */
    size_t bytesNeededTotal() const {
        return this->bytesNeededPerRow() * fBitmap.height();
    }

    /**
     * Writes the entire bitmap into dstBuffer, using the already-specified
     * pixelFormat. Returns true if successful.
     *
     * dstBufferSize is the maximum allowable bytes to write into dstBuffer;
     * if that is not large enough to hold the entire bitmap, then this
     * will fail immediately and return false.
     * We force the caller to pass this in to avoid buffer overruns in
     * unanticipated cases.
     *
     * All pixels for all rows will be written into dstBuffer as a
     * single contiguous blob (no skipped pixels at the end of each
     * row).
     */
    bool copyBitmapToPixelBuffer (void *dstBuffer, size_t dstBufferSize) const;

private:
    const SkBitmap& fBitmap;
    const PixelFormat fPixelFormat;
};

#endif
