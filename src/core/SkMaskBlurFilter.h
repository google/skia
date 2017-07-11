/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMaskFilter_DEFINED
#define SkBlurMaskFilter_DEFINED

#include <algorithm>
#include <memory>

#include "SkMask.h"
#include "SkTypes.h"

// Spec: https://drafts.fxtf.org/filters/#feGaussianBlurElement
class SkMaskBlurFilter {
public:
    class FilterInfo {
    public:
        explicit FilterInfo(double sigma);
        uint64_t weight() const;
        uint32_t borderSize() const;
        size_t   diameter(uint8_t) const;
        uint64_t scaledWeight() const;

    private:
        const uint32_t fFilterWindow;
        const uint64_t fScaledWeight;
    };

    SkMaskBlurFilter(double sigmaW, double sigmaH);

    SkIPoint blur(SkMask* dst, const SkMask& src) const;

    void blurOneScan(FilterInfo gen,
                     const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                           uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const;

private:
    size_t bufferSize(uint8_t bufferPass) const {
        return std::max(fInfoW.diameter(bufferPass), fInfoH.diameter(bufferPass)) - 1;
    }

    const FilterInfo            fInfoW,
                                fInfoH;
    std::unique_ptr<uint32_t[]> fBuffer0,
                                fBuffer1,
                                fBuffer2;
};

#endif  // SkBlurMaskFilter_DEFINED
