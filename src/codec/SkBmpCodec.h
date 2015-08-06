/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpCodec_DEFINED
#define SkBmpCodec_DEFINED

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkMaskSwizzler.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkTypes.h"

/*
 * This class enables code sharing between its bmp codec subclasses.  The
 * subclasses actually do the work.
 */
class SkBmpCodec : public SkCodec {
public:

    /*
     * Describes if rows of the input start at the top or bottom of the image
     */
    enum RowOrder {
        kTopDown_RowOrder,
        kBottomUp_RowOrder
    };

    /*
     * Checks the start of the stream to see if the image is a bmp
     */
    static bool IsBmp(SkStream*);

    /*
     * Assumes IsBmp was called and returned true
     * Creates a bmp decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*);

    /*
     * Creates a bmp decoder for a bmp embedded in ico
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromIco(SkStream*);

protected:

    SkBmpCodec(const SkImageInfo& info, SkStream* stream, uint16_t bitsPerPixel,
            RowOrder rowOrder);

    SkEncodedFormat onGetEncodedFormat() const override { return kBMP_SkEncodedFormat; }

    /*
     * Read enough of the stream to initialize the SkBmpCodec. Returns a bool
     * representing success or failure. If it returned true, and codecOut was
     * not NULL, it will be set to a new SkBmpCodec.
     * Does *not* take ownership of the passed in SkStream.
     */
    static bool ReadHeader(SkStream*, bool inIco, SkCodec** codecOut);

    /*
     * Rewinds the image stream if necessary
     */
    bool handleRewind(bool inIco);

    /*
     * Get the destination row to start filling from
     * Used to fill the remainder of the image on incomplete input for bmps
     * This is tricky since bmps may be kTopDown or kBottomUp.  For kTopDown,
     * we start filling from where we left off, but for kBottomUp we start
     * filling at the top of the image.
     */
    void* getDstStartRow(void* dst, size_t dstRowBytes, int32_t y) const;

    /*
     * Compute the number of colors in the color table
     */
    uint32_t computeNumColors(uint32_t numColors);

    /*
     * Accessors used by subclasses
     */
    uint16_t bitsPerPixel() const { return fBitsPerPixel; }
    RowOrder rowOrder() const { return fRowOrder; }

private:

    /*
     * Creates a bmp decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*, bool inIco);

    const uint16_t fBitsPerPixel;
    const RowOrder fRowOrder;

    typedef SkCodec INHERITED;
};

#endif
