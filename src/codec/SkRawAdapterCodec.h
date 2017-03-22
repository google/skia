/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRawAdapterCodec_DEFINED
#define SkRawAdapterCodec_DEFINED

#include "SkAndroidCodec.h"
#include "SkCodec.h"
#include "SkEncodedImageFormat.h"
#include "SkRawCodec.h"
#include "SkStream.h"
#include "SkTypes.h"

/**
 *  This class implements the functionality of SkAndroidCodec. It uses an
 *  SkRawCodec.
 */
class SkRawAdapterCodec : public SkAndroidCodec {
public:

    explicit SkRawAdapterCodec(SkRawCodec*);

    ~SkRawAdapterCodec() override {}

protected:

    SkISize onGetSampledDimensions(int sampleSize) const override;

    bool onGetSupportedSubset(SkIRect* /*desiredSubset*/) const override { return false; }

    SkCodec::Result onGetAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions& options) override;

private:

    typedef SkAndroidCodec INHERITED;
};
#endif // SkRawAdapterCodec_DEFINED
