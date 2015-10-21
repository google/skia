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
            AndroidOptions& options) override;

private:

    /**
     *  This fulfills the same contract as onGetAndroidPixels().
     *
     *  We call this function from onGetAndroidPixels() if we have determined
     *  that fCodec does not support the requested scale, and we need to
     *  provide the scale by sampling.
     */
    SkCodec::Result sampledDecode(const SkImageInfo& info, void* pixels, size_t rowBytes,
            AndroidOptions& options);

    SkAutoTDelete<SkCodec> fCodec;

    typedef SkAndroidCodec INHERITED;
};
#endif // SkSampledCodec_DEFINED
