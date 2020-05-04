/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BitmapRegionDecoder_DEFINED
#define BitmapRegionDecoder_DEFINED

#include "client_utils/android/BRDAllocator.h"
// Temporary, until Android switches to the new class.
#include "include/android/SkBitmapRegionDecoder.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"

namespace android {
namespace skia {

class BitmapRegionDecoder final : public SkBitmapRegionDecoder {
public:
    static std::unique_ptr<BitmapRegionDecoder> Make(sk_sp<SkData> data);

    bool decodeRegion(SkBitmap* bitmap, BRDAllocator* allocator,
                      const SkIRect& desiredSubset, int sampleSize,
                      SkColorType colorType, bool requireUnpremul,
                      sk_sp<SkColorSpace> prefColorSpace) override;

    SkEncodedImageFormat getEncodedFormat() override { return fCodec->getEncodedFormat(); }

    SkColorType computeOutputColorType(SkColorType requestedColorType) override {
        return fCodec->computeOutputColorType(requestedColorType);
    }

    sk_sp<SkColorSpace> computeOutputColorSpace(SkColorType outputColorType,
            sk_sp<SkColorSpace> prefColorSpace = nullptr) override {
        return fCodec->computeOutputColorSpace(outputColorType, prefColorSpace);
    }

private:
    BitmapRegionDecoder(std::unique_ptr<SkAndroidCodec> codec);

    std::unique_ptr<SkAndroidCodec> fCodec;

    typedef SkBitmapRegionDecoder INHERITED;

};

} // namespace skia
} // namespace android
#endif  // BitmapRegionDecoder_DEFINED
