/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkScaledCodec_DEFINED
#define SkScaledCodec_DEFINED

#include "SkCodec.h"
#include "SkScanlineDecoder.h"

class SkScanlineDecoder;
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

    /**
     * returns whether a destination's dimensions are supported for down sampling
     */
    static bool DimensionsSupportedForSampling(const SkImageInfo& srcInfo, 
                                               const SkImageInfo& dstInfo) {
        // heights must be equal as no native y sampling is supported
        if (dstInfo.height() != srcInfo.height()) {
            return false;
        }
        // only support down sampling, dstWidth cannot be larger that srcWidth
        if(dstInfo.width() > srcInfo.width()) {
            return false;
        }
        return true;   
    }

    static void ComputeSampleSize(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo,
                                  int* sampleSizeX, int* sampleSizeY);

protected:
    /**
     * Recommend a set of destination dimensions given a requested scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, SkPMColor*, int*)
            override;
    SkEncodedFormat onGetEncodedFormat() const override {
        return fScanlineDecoder->getEncodedFormat();
    }

    bool onReallyHasAlpha() const override {
        return fScanlineDecoder->reallyHasAlpha();
    }

private:

    SkAutoTDelete<SkScanlineDecoder> fScanlineDecoder;

    explicit SkScaledCodec(SkScanlineDecoder*);

    typedef SkCodec INHERITED;
};
#endif // SkScaledCodec_DEFINED
