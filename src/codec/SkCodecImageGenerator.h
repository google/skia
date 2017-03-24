/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCodecImageGenerator_DEFINED
#define SkCodecImageGenerator_DEFINED

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkData.h"
#include "SkImageGenerator.h"

class SkCodecImageGenerator : public SkImageGenerator {
public:
    /*
     * If this data represents an encoded image that we know how to decode,
     * return an SkCodecImageGenerator.  Otherwise return nullptr.
     */
    static std::unique_ptr<SkImageGenerator> MakeFromEncodedCodec(sk_sp<SkData>);

protected:
    SkData* onRefEncodedData(GrContext* ctx) override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts)
                     override;

    bool onQueryYUV8(SkYUVSizeInfo*, SkYUVColorSpace*) const override;

    bool onGetYUV8Planes(const SkYUVSizeInfo&, void* planes[3]) override;

private:
    /*
     * Takes ownership of codec
     */
    SkCodecImageGenerator(SkCodec* codec, sk_sp<SkData>);

    std::unique_ptr<SkCodec> fCodec;
    sk_sp<SkData> fData;
    sk_sp<SkColorTable> fColorTable;

    typedef SkImageGenerator INHERITED;
};
#endif  // SkCodecImageGenerator_DEFINED
