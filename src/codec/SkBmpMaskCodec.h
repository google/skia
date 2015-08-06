/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkImageInfo.h"
#include "SkMaskSwizzler.h"
#include "SkTypes.h"

/*
 * This class implements the decoding for bmp images using bit masks
 */
class SkBmpMaskCodec : public SkBmpCodec {
public:

    /*
     * Creates an instance of the decoder
     *
     * Called only by SkBmpCodec::NewFromStream
     * There should be no other callers despite this being public
     *
     * @param srcInfo contains the source width and height
     * @param stream the stream of encoded image data
     * @param bitsPerPixel the number of bits used to store each pixel
     * @param masks color masks for certain bmp formats
     * @param rowOrder indicates whether rows are ordered top-down or bottom-up
     */
    SkBmpMaskCodec(const SkImageInfo& srcInfo, SkStream* stream,
                   uint16_t bitsPerPixel, SkMasks* masks, RowOrder rowOrder);

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst,
                       size_t dstRowBytes, const Options&, SkPMColor*,
                       int*) override;

private:

    bool initializeSwizzler(const SkImageInfo& dstInfo);

    Result decode(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
                  const Options& opts);

    SkAutoTDelete<SkMasks>              fMasks;        // owned
    SkAutoTDelete<SkMaskSwizzler>       fMaskSwizzler;
    SkAutoTDeleteArray<uint8_t>         fSrcBuffer;

    typedef SkBmpCodec INHERITED;
};
