/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpStandardCodec_DEFINED
#define SkBmpStandardCodec_DEFINED

#include "SkBmpBaseCodec.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"
#include "SkTypes.h"

/*
 * This class implements the decoding for bmp images that use "standard" modes,
 * which essentially means they do not contain bit masks or RLE codes.
 */
class SkBmpStandardCodec : public SkBmpBaseCodec {
public:

    /*
     * Creates an instance of the decoder
     *
     * Called only by SkBmpCodec::NewFromStream
     * There should be no other callers despite this being public
     *
     * @param info contains properties of the encoded data
     * @param stream the stream of encoded image data
     * @param bitsPerPixel the number of bits used to store each pixel
     * @param numColors the number of colors in the color table
     * @param bytesPerColor the number of bytes in the stream used to represent
                            each color in the color table
     * @param offset the offset of the image pixel data from the end of the
     *               headers
     * @param rowOrder indicates whether rows are ordered top-down or bottom-up
     * @param isOpaque indicates if the bmp itself is opaque (before applying
     *                 the icp mask, if there is one)
     * @param inIco    indicates if the bmp is embedded in an ico file
     */
    SkBmpStandardCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
            uint16_t bitsPerPixel, uint32_t numColors, uint32_t bytesPerColor,
            uint32_t offset, SkCodec::SkScanlineOrder rowOrder, bool isOpaque,
            bool inIco);

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst,
                       size_t dstRowBytes, const Options&,
                       int*) override;

    bool onInIco() const override {
        return fInIco;
    }

    SkCodec::Result onPrepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options) override;


    uint64_t onGetFillValue(const SkImageInfo&) const override;

    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler);
        return fSwizzler.get();
    }

private:

    /*
     * Creates the color table
     */
    bool createColorTable(SkColorType colorType, SkAlphaType alphaType);

    void initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts);

    int decodeRows(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
            const Options& opts) override;

    /*
     * @param stream This may be a pointer to the stream owned by the parent SkCodec
     *               or a sub-stream of the stream owned by the parent SkCodec.
     *               Either way, this stream is unowned.
     */
    void decodeIcoMask(SkStream* stream, const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes);

    sk_sp<SkColorTable>         fColorTable;
    // fNumColors is the number specified in the header, or 0 if not present in the header.
    const uint32_t              fNumColors;
    const uint32_t              fBytesPerColor;
    const uint32_t              fOffset;
    std::unique_ptr<SkSwizzler> fSwizzler;
    const bool                  fIsOpaque;
    const bool                  fInIco;
    const size_t                fAndMaskRowBytes; // only used for fInIco decodes

    typedef SkBmpBaseCodec INHERITED;
};
#endif  // SkBmpStandardCodec_DEFINED
