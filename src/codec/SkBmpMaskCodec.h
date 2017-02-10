/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBmpMaskCodec_DEFINED
#define SkBmpMaskCodec_DEFINED

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
     * @param info contains properties of the encoded data
     * @param stream the stream of encoded image data
     * @param bitsPerPixel the number of bits used to store each pixel
     * @param masks color masks for certain bmp formats
     * @param rowOrder indicates whether rows are ordered top-down or bottom-up
     */
    SkBmpMaskCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
            uint16_t bitsPerPixel, SkMasks* masks,
            SkCodec::SkScanlineOrder rowOrder);

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst,
                       size_t dstRowBytes, const Options&, SkPMColor*,
                       int*, int*) override;

    SkCodec::Result onPrepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options, SkPMColor inputColorPtr[],
            int* inputColorCount) override;

private:

    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fMaskSwizzler);
        return fMaskSwizzler.get();
    }

    int decodeRows(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
            const Options& opts) override;

    std::unique_ptr<SkMasks>        fMasks;
    std::unique_ptr<SkMaskSwizzler> fMaskSwizzler;
    std::unique_ptr<uint8_t[]>      fSrcBuffer;

    typedef SkBmpCodec INHERITED;
};
#endif  // SkBmpMaskCodec_DEFINED
