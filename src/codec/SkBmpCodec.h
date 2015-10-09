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
            SkCodec::SkScanlineOrder rowOrder);

    SkEncodedFormat onGetEncodedFormat() const override { return kBMP_SkEncodedFormat; }

    /*
     * Read enough of the stream to initialize the SkBmpCodec. Returns a bool
     * representing success or failure. If it returned true, and codecOut was
     * not nullptr, it will be set to a new SkBmpCodec.
     * Does *not* take ownership of the passed in SkStream.
     */
    static bool ReadHeader(SkStream*, bool inIco, SkCodec** codecOut);

    bool onRewind() override;

    /*
     * Returns whether this BMP is part of an ICO image.
     */
    bool inIco() const {
        return this->onInIco();
    }

    virtual bool onInIco() const {
        return false;
    }

    /*
     * Get the destination row number corresponding to the encoded row number.
     * For kTopDown, we simply return y, but for kBottomUp, the rows will be
     * decoded in reverse order.
     *
     * @param y      Iterates from 0 to height, indicating the current row.
     * @param height The height of the current subset of the image that we are
     *               decoding.  This is generally equal to the full height
     *               when we want to decode the full or one when we are
     *               sampling.
     */
    int32_t getDstRow(int32_t y, int32_t height) const;

    /*
     * Compute the number of colors in the color table
     */
    uint32_t computeNumColors(uint32_t numColors);

    /*
     * Accessors used by subclasses
     */
    uint16_t bitsPerPixel() const { return fBitsPerPixel; }
    SkScanlineOrder onGetScanlineOrder() const override { return fRowOrder; }

    /*
     * To be overriden by bmp subclasses, which provide unique implementations.
     * Performs subclass specific setup.
     *
     * @param dstInfo         Contains output information.  Height specifies
     *                        the total number of rows that will be decoded.
     * @param options         Additonal options to pass to the decoder.
     * @param inputColorPtr   Client-provided memory for a color table.  Must
     *                        be enough for 256 colors.  This will be
     *                        populated with colors if the encoded image uses
     *                        a color table.
     * @param inputColorCount If the encoded image uses a color table, this
     *                        will be set to the number of colors in the
     *                        color table.
     */
    virtual SkCodec::Result prepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options, SkPMColor inputColorPtr[],
            int* inputColorCount) = 0;

private:

    /*
     * Creates a bmp decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*, bool inIco);

    /*
     * Decodes the next dstInfo.height() lines.
     *
     * onGetPixels() uses this for full image decodes.
     * SkScaledCodec::onGetPixels() uses the scanline decoder to call this with
     * dstInfo.height() = 1, in order to implement sampling.
     * A potential future use is to allow the caller to decode a subset of the
     * lines in the image.
     *
     * @param dstInfo     Contains output information.  Height specifies the
     *                    number of rows to decode at this time.
     * @param dst         Memory location to store output pixels
     * @param dstRowBytes Bytes in a row of the destination
     * @return            Number of rows successfully decoded
     */
    virtual int decodeRows(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
            const Options& opts) = 0;

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const SkCodec::Options&,
            SkPMColor inputColorPtr[], int* inputColorCount) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    // TODO(msarett): Override default skipping with something more clever.

    const uint16_t          fBitsPerPixel;
    const SkScanlineOrder   fRowOrder;

    typedef SkCodec INHERITED;
};

#endif
