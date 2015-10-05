/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkScaledCodec_DEFINED
#define SkScaledCodec_DEFINED

#include "SkCodec.h"

class SkStream;

/**
 * This class implements scaling, by sampling scanlines in the y direction.
 * x-wise sampling is implemented in the swizzler, when getScanlines() is called.
 */
class SkScaledCodec : public SkCodec {
public:
    static SkCodec* NewFromStream(SkStream*);
    static SkCodec* NewFromData(SkData*);

    virtual ~SkScaledCodec();

    static void ComputeSampleSize(const SkISize& dstDim, const SkISize& srcDim,
                                  int* sampleSizeX, int* sampleSizeY);

protected:
    bool onRewind() override;

    /**
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;
    bool onDimensionsSupported(const SkISize&) override;

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override {
        return fCodec->getEncodedFormat();
    }

    bool onReallyHasAlpha() const override {
        return fCodec->reallyHasAlpha();
    }

private:

    SkAutoTDelete<SkCodec> fCodec;

    explicit SkScaledCodec(SkCodec*);

    typedef SkCodec INHERITED;
};
#endif // SkScaledCodec_DEFINED
