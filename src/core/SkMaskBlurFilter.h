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

// Implement a single channel Gaussian blur. The specifics for implementation are taken from:
// https://drafts.fxtf.org/filters/#feGaussianBlurElement
class SkMaskBlurFilter {
public:
    // Given a filter specified by sigma, generate various quantities.
    class FilterInfo {
    public:
        explicit FilterInfo(double sigma);

        // The final weight to divide by given a box size calculated from sigma accumulated for
        // all three passes. For example, if the box size is 5, then the final weight for all
        // three passes is 5^3 or 125.
        uint64_t weight() const;

        // The distance between the first value of the dst and the first value of the src.
        uint32_t borderSize() const;

        // The size of the box filter.
        size_t   diameter(uint8_t) const;

        // A factor used to simulate division using multiplication and shift.
        uint64_t scaledWeight() const;

        // Returned when sigma < 2.
        bool isSmall() const;

    private:
        const bool     fIsSmall;
        const uint32_t fFilterWindow;
        const uint64_t fWeight;
        const uint64_t fScaledWeight;
    };

    // Create an object suitable for filtering an SkMask using a filter with width sigmaW and
    // height sigmaH.
    SkMaskBlurFilter(double sigmaW, double sigmaH);

    // Given a src SkMask, generate dst SkMask returning the border width and height.
    SkIPoint blur(const SkMask& src, SkMask* dst) const;

private:
    size_t bufferSize(uint8_t bufferPass) const;

    void blurOneScan(FilterInfo gen,
                     const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                           uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const;

    void blurOneScanBox(FilterInfo gen,
                        const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                              uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const;

    void blurOneScanGauss(FilterInfo gen,
                          const uint8_t* src, size_t srcStride, const uint8_t* srcEnd,
                                uint8_t* dst, size_t dstStride,       uint8_t* dstEnd) const;


    const FilterInfo            fInfoW,
                                fInfoH;
    std::unique_ptr<uint32_t[]> fBuffer0,
                                fBuffer1,
                                fBuffer2;
};

#endif  // SkBlurMaskFilter_DEFINED
