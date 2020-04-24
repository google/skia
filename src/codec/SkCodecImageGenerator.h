/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCodecImageGenerator_DEFINED
#define SkCodecImageGenerator_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"

class SkCodecImageGenerator : public SkImageGenerator {
public:
    /*
     * If this data represents an encoded image that we know how to decode,
     * return an SkCodecImageGenerator.  Otherwise return nullptr.
     */
    static std::unique_ptr<SkImageGenerator> MakeFromEncodedCodec(sk_sp<SkData>);

    static std::unique_ptr<SkImageGenerator> MakeFromCodec(std::unique_ptr<SkCodec>);

protected:
    sk_sp<SkData> onRefEncodedData() override;

    bool onGetPixels(
        const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts) override;

    bool onQueryYUVA8(
        SkYUVASizeInfo*, SkYUVAIndex[SkYUVAIndex::kIndexCount], SkYUVColorSpace*) const override;

    bool onGetYUVA8Planes(const SkYUVASizeInfo&, const SkYUVAIndex[SkYUVAIndex::kIndexCount],
                          void* planes[]) override;

private:
    /*
     * Takes ownership of codec
     */
    SkCodecImageGenerator(std::unique_ptr<SkCodec>, sk_sp<SkData>);

    std::unique_ptr<SkCodec> fCodec;
    sk_sp<SkData> fData;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkCodecImageGenerator_DEFINED
