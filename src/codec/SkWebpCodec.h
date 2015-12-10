/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWebpCodec_DEFINED
#define SkWebpCodec_DEFINED

#include "SkCodec.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

class SkStream;

static const size_t WEBP_VP8_HEADER_SIZE = 30;

class SkWebpCodec final : public SkCodec {
public:
    // Assumes IsWebp was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsWebp(const void*, size_t);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kWEBP_SkEncodedFormat; }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    bool onGetValidSubset(SkIRect* /* desiredSubset */) const override;
private:
    SkWebpCodec(const SkImageInfo&, SkStream*);

    typedef SkCodec INHERITED;
};
#endif // SkWebpCodec_DEFINED
