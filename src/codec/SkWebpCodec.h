/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWebpCodec_DEFINED
#define SkWebpCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkEncodedImageFormat.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

class SkStream;
extern "C" {
    struct WebPDemuxer;
    void WebPDemuxDelete(WebPDemuxer* dmux);
}

static const size_t WEBP_VP8_HEADER_SIZE = 30;

class SkWebpCodec final : public SkCodec {
public:
    // Assumes IsWebp was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsWebp(const void*, size_t);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*, int*)
            override;
    SkEncodedImageFormat onGetEncodedFormat() const override { return SkEncodedImageFormat::kWEBP; }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    bool onGetValidSubset(SkIRect* /* desiredSubset */) const override;
private:
    SkWebpCodec(int width, int height, const SkEncodedInfo&, sk_sp<SkColorSpace>, SkStream*,
                WebPDemuxer*, sk_sp<SkData>);

    SkAutoTCallVProc<WebPDemuxer, WebPDemuxDelete> fDemux;

    // fDemux has a pointer into this data.
    // This should not be freed until the decode is completed.
    sk_sp<SkData> fData;

    typedef SkCodec INHERITED;
};
#endif // SkWebpCodec_DEFINED
