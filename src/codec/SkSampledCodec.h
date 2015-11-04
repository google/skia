/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSampledCodec_DEFINED
#define SkSampledCodec_DEFINED

#include "SkAndroidCodec.h"
#include "SkCodec.h"

/**
 *  This class implements the functionality of SkAndroidCodec.  Scaling will
 *  be provided by sampling if it cannot be provided by fCodec.
 */
class SkSampledCodec : public SkAndroidCodec {
public:

    explicit SkSampledCodec(SkCodec*);

    virtual ~SkSampledCodec() {}

protected:

    SkEncodedFormat onGetEncodedFormat() const override { return fCodec->getEncodedFormat(); };

    SkISize onGetSampledDimensions(int sampleSize) const override;

    bool onGetSupportedSubset(SkIRect* desiredSubset) const override { return true; }

    SkCodec::Result onGetAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions& options) override;

private:
    /**
     *  Find the best way to account for native scaling.
     *
     *  Return a size that fCodec can scale to, and adjust sampleSize to finish scaling.
     *
     *  @param sampleSize As an input, the requested sample size.
     *                    As an output, sampling needed after letting fCodec
     *                    scale to the returned dimensions.
     *  @param nativeSampleSize Optional output parameter. Will be set to the
     *                          effective sample size done by fCodec.
     *  @return SkISize The size that fCodec should scale to.
     */
    SkISize accountForNativeScaling(int* sampleSize, int* nativeSampleSize = nullptr) const;

    /**
     *  This fulfills the same contract as onGetAndroidPixels().
     *
     *  We call this function from onGetAndroidPixels() if we have determined
     *  that fCodec does not support the requested scale, and we need to
     *  provide the scale by sampling.
     */
    SkCodec::Result sampledDecode(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions& options);

    SkAutoTDelete<SkCodec> fCodec;

    typedef SkAndroidCodec INHERITED;
};
#endif // SkSampledCodec_DEFINED
