/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkWebpAdapterCodec_DEFINED
#define SkWebpAdapterCodec_DEFINED

#include "SkAndroidCodec.h"
#include "SkWebpCodec.h"

/**
 *  This class implements the functionality of SkAndroidCodec.  It uses an
 *  SkWebpCodec.
 */
class SkWebpAdapterCodec : public SkAndroidCodec {
public:

    explicit SkWebpAdapterCodec(SkWebpCodec*);

    virtual ~SkWebpAdapterCodec() {}

protected:

    SkEncodedFormat onGetEncodedFormat() const override { return kWEBP_SkEncodedFormat; };

    SkISize onGetSampledDimensions(int sampleSize) const override;

    bool onGetSupportedSubset(SkIRect* desiredSubset) const override;

    SkCodec::Result onGetAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions& options) override;

private:

    SkAutoTDelete<SkWebpCodec> fCodec;

    typedef SkAndroidCodec INHERITED;
};
#endif // SkWebpAdapterCodec_DEFINED
