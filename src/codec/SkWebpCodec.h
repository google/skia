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

class SkWebpCodec final : public SkCodec {
public:
    // Assumes IsWebp was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsWebp(SkStream*);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override { return kWEBP_SkEncodedFormat; }

    bool onReallyHasAlpha() const override {
        return this->getInfo().alphaType() != kOpaque_SkAlphaType;
    }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onGetValidSubset(SkIRect* /* desiredSubset */) const override;
private:
    SkWebpCodec(const SkImageInfo&, SkStream*);

    typedef SkCodec INHERITED;
};
#endif // SkWebpCodec_DEFINED
