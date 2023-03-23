/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRawCodec_DEFINED
#define SkRawCodec_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"

#include <cstddef>
#include <memory>

class SkDngImage;
class SkStream;
struct SkImageInfo;

/*
 *
 * This class implements the decoding for RAW images
 *
 */
class SkRawCodec : public SkCodec {
public:

    /*
     * Creates a RAW decoder
     * Takes ownership of the stream
     */
    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkRawCodec() override;

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kDNG;
    }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    // SkCodec only applies the colorXform if it's necessary for color space
    // conversion. SkRawCodec will always convert, so tell SkCodec not to.
    bool usesColorXform() const override { return false; }

private:

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream, takes ownership of dngImage.
     */
    SkRawCodec(SkDngImage* dngImage);

    std::unique_ptr<SkDngImage> fDngImage;

    using INHERITED = SkCodec;
};

#endif
