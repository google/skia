/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAndroidCodecAdapter_DEFINED
#define SkAndroidCodecAdapter_DEFINED

#include "include/codec/SkAndroidCodec.h"

/**
 *  This class wraps SkCodec to implement the functionality of SkAndroidCodec.
 *  The underlying SkCodec implements sampled decodes.  SkCodec's that do not
 *  implement that are wrapped with SkSampledCodec instead.
 */
class SkAndroidCodecAdapter : public SkAndroidCodec {
public:

    explicit SkAndroidCodecAdapter(SkCodec*, ExifOrientationBehavior);

    ~SkAndroidCodecAdapter() override {}

protected:

    SkISize onGetSampledDimensions(int sampleSize) const override;

    bool onGetSupportedSubset(SkIRect* desiredSubset) const override;

    SkCodec::Result onGetAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions& options) override;

private:

    typedef SkAndroidCodec INHERITED;
};
#endif // SkAndroidCodecAdapter_DEFINED
