/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBmpMaskCodec_DEFINED
#define SkBmpMaskCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkTypes.h"
#include "src/codec/SkBmpBaseCodec.h"
#include "src/codec/SkMaskSwizzler.h"
#include "src/core/SkMasks.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkSampler;
class SkStream;
struct SkEncodedInfo;
struct SkImageInfo;

/*
 * This class implements the decoding for bmp images using bit masks
 */
class SkBmpMaskCodec : public SkBmpBaseCodec {
public:

    /*
     * Creates an instance of the decoder
     *
     * Called only by SkBmpCodec::MakeFromStream
     * There should be no other callers despite this being public
     *
     * @param info contains properties of the encoded data
     * @param stream the stream of encoded image data
     * @param bitsPerPixel the number of bits used to store each pixel
     * @param masks color masks for certain bmp formats
     * @param rowOrder indicates whether rows are ordered top-down or bottom-up
     */
    SkBmpMaskCodec(SkEncodedInfo&& info, std::unique_ptr<SkStream>,
            uint16_t bitsPerPixel, SkMasks* masks,
            SkCodec::SkScanlineOrder rowOrder);

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst,
                       size_t dstRowBytes, const Options&,
                       int*) override;

    SkCodec::Result onPrepareToDecode(const SkImageInfo& dstInfo,
            const SkCodec::Options& options) override;

private:

    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fMaskSwizzler);
        return fMaskSwizzler.get();
    }

    int decodeRows(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes,
            const Options& opts) override;

    std::unique_ptr<SkMasks>        fMasks;
    std::unique_ptr<SkMaskSwizzler> fMaskSwizzler;

    using INHERITED = SkBmpBaseCodec;
};
#endif  // SkBmpMaskCodec_DEFINED
