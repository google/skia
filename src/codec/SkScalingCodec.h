/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkScalingCodec_DEFINED
#define SkScalingCodec_DEFINED

#include "include/codec/SkCodec.h"

// Helper class for an SkCodec that supports arbitrary downscaling.
class SkScalingCodec : public SkCodec {
protected:
    SkScalingCodec(SkEncodedInfo&& info, XformFormat srcFormat, std::unique_ptr<SkStream> stream,
                    SkEncodedOrigin origin = kTopLeft_SkEncodedOrigin)
        : INHERITED(std::move(info), srcFormat, std::move(stream), origin) {}

    SkISize onGetScaledDimensions(float desiredScale) const override {
        SkISize dim = this->dimensions();
        // SkCodec treats zero dimensional images as errors, so the minimum size
        // that we will recommend is 1x1.
        dim.fWidth = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fWidth));
        dim.fHeight = SkTMax(1, SkScalarRoundToInt(desiredScale * dim.fHeight));
        return dim;
    }

    bool onDimensionsSupported(const SkISize& requested) override {
        SkISize dim = this->dimensions();
        int w = requested.width();
        int h = requested.height();
        return 1 <= w && w <= dim.width() && 1 <= h && h <= dim.height();
    }

private:
    typedef SkCodec INHERITED;
};

#endif  // SkScalingCodec_DEFINED
