/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapScaler.h"
#include "SkBitmapFilter.h"
#include "SkConvolver.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "SkRect.h"
#include "SkTArray.h"

// SkResizeFilter ----------------------------------------------------------------

// Encapsulates computation and storage of the filters required for one complete
// resize operation.
class SkResizeFilter {
public:
    SkResizeFilter(SkBitmapScaler::ResizeMethod method,
                   int srcFullWidth, int srcFullHeight,
                   float destWidth, float destHeight,
                   const SkRect& destSubset,
                   const SkConvolutionProcs& convolveProcs);
    ~SkResizeFilter() { delete fBitmapFilter; }

    // Returns the filled filter values.
    const SkConvolutionFilter1D& xFilter() { return fXFilter; }
    const SkConvolutionFilter1D& yFilter() { return fYFilter; }

private:

    SkBitmapFilter* fBitmapFilter;

    // Computes one set of filters either horizontally or vertically. The caller
    // will specify the "min" and "max" rather than the bottom/top and
    // right/bottom so that the same code can be re-used in each dimension.
    //
    // |srcDependLo| and |srcDependSize| gives the range for the source
    // depend rectangle (horizontally or vertically at the caller's discretion
    // -- see above for what this means).
    //
    // Likewise, the range of destination values to compute and the scale factor
    // for the transform is also specified.

    void computeFilters(int srcSize,
                        float destSubsetLo, float destSubsetSize,
                        float scale,
                        SkConvolutionFilter1D* output,
                        const SkConvolutionProcs& convolveProcs);

    SkConvolutionFilter1D fXFilter;
    SkConvolutionFilter1D fYFilter;
};

SkResizeFilter::SkResizeFilter(SkBitmapScaler::ResizeMethod method,
                               int srcFullWidth, int srcFullHeight,
                               float destWidth, float destHeight,
                               const SkRect& destSubset,
                               const SkConvolutionProcs& convolveProcs) {

    SkASSERT(method >= SkBitmapScaler::RESIZE_FirstMethod &&
             method <= SkBitmapScaler::RESIZE_LastMethod);

    fBitmapFilter = nullptr;
    switch(method) {
        case SkBitmapScaler::RESIZE_BOX:
            fBitmapFilter = new SkBoxFilter;
            break;
        case SkBitmapScaler::RESIZE_TRIANGLE:
            fBitmapFilter = new SkTriangleFilter;
            break;
        case SkBitmapScaler::RESIZE_MITCHELL:
            fBitmapFilter = new SkMitchellFilter;
            break;
        case SkBitmapScaler::RESIZE_HAMMING:
            fBitmapFilter = new SkHammingFilter;
            break;
        case SkBitmapScaler::RESIZE_LANCZOS3:
            fBitmapFilter = new SkLanczosFilter;
            break;
    }


    float scaleX = destWidth / srcFullWidth;
    float scaleY = destHeight / srcFullHeight;

    this->computeFilters(srcFullWidth, destSubset.fLeft, destSubset.width(),
                         scaleX, &fXFilter, convolveProcs);
    if (srcFullWidth == srcFullHeight &&
        destSubset.fLeft == destSubset.fTop &&
        destSubset.width() == destSubset.height()&&
        scaleX == scaleY) {
        fYFilter = fXFilter;
    } else {
        this->computeFilters(srcFullHeight, destSubset.fTop, destSubset.height(),
                          scaleY, &fYFilter, convolveProcs);
    }
}

// TODO(egouriou): Take advantage of periods in the convolution.
// Practical resizing filters are periodic outside of the border area.
// For Lanczos, a scaling by a (reduced) factor of p/q (q pixels in the
// source become p pixels in the destination) will have a period of p.
// A nice consequence is a period of 1 when downscaling by an integral
// factor. Downscaling from typical display resolutions is also bound
// to produce interesting periods as those are chosen to have multiple
// small factors.
// Small periods reduce computational load and improve cache usage if
// the coefficients can be shared. For periods of 1 we can consider
// loading the factors only once outside the borders.
void SkResizeFilter::computeFilters(int srcSize,
                                  float destSubsetLo, float destSubsetSize,
                                  float scale,
                                  SkConvolutionFilter1D* output,
                                  const SkConvolutionProcs& convolveProcs) {
  float destSubsetHi = destSubsetLo + destSubsetSize;  // [lo, hi)

  // When we're doing a magnification, the scale will be larger than one. This
  // means the destination pixels are much smaller than the source pixels, and
  // that the range covered by the filter won't necessarily cover any source
  // pixel boundaries. Therefore, we use these clamped values (max of 1) for
  // some computations.
  float clampedScale = SkTMin(1.0f, scale);

  // This is how many source pixels from the center we need to count
  // to support the filtering function.
  float srcSupport = fBitmapFilter->width() / clampedScale;

  float invScale = 1.0f / scale;

  SkSTArray<64, float, true> filterValuesArray;
  SkSTArray<64, SkConvolutionFilter1D::ConvolutionFixed, true> fixedFilterValuesArray;

  // Loop over all pixels in the output range. We will generate one set of
  // filter values for each one. Those values will tell us how to blend the
  // source pixels to compute the destination pixel.

  // This is the pixel in the source directly under the pixel in the dest.
  // Note that we base computations on the "center" of the pixels. To see
  // why, observe that the destination pixel at coordinates (0, 0) in a 5.0x
  // downscale should "cover" the pixels around the pixel with *its center*
  // at coordinates (2.5, 2.5) in the source, not those around (0, 0).
  // Hence we need to scale coordinates (0.5, 0.5), not (0, 0).
  destSubsetLo = SkScalarFloorToScalar(destSubsetLo);
  destSubsetHi = SkScalarCeilToScalar(destSubsetHi);
  float srcPixel = (destSubsetLo + 0.5f) * invScale;
  int destLimit = SkScalarTruncToInt(destSubsetHi - destSubsetLo);
  output->reserveAdditional(destLimit, SkScalarCeilToInt(destLimit * srcSupport * 2));
  for (int destI = 0; destI < destLimit; srcPixel += invScale, destI++)
  {
    // Compute the (inclusive) range of source pixels the filter covers.
    float srcBegin = SkTMax(0.f, SkScalarFloorToScalar(srcPixel - srcSupport));
    float srcEnd = SkTMin(srcSize - 1.f, SkScalarCeilToScalar(srcPixel + srcSupport));

    // Compute the unnormalized filter value at each location of the source
    // it covers.

    // Sum of the filter values for normalizing.
    // Distance from the center of the filter, this is the filter coordinate
    // in source space. We also need to consider the center of the pixel
    // when comparing distance against 'srcPixel'. In the 5x downscale
    // example used above the distance from the center of the filter to
    // the pixel with coordinates (2, 2) should be 0, because its center
    // is at (2.5, 2.5).
    float destFilterDist = (srcBegin + 0.5f - srcPixel) * clampedScale;
    int filterCount = SkScalarTruncToInt(srcEnd - srcBegin) + 1;
    if (filterCount <= 0) {
        // true when srcSize is equal to srcPixel - srcSupport; this may be a bug
        return;
    }
    filterValuesArray.reset(filterCount);
    float filterSum = fBitmapFilter->evaluate_n(destFilterDist, clampedScale, filterCount,
                                                filterValuesArray.begin());

    // The filter must be normalized so that we don't affect the brightness of
    // the image. Convert to normalized fixed point.
    int fixedSum = 0;
    fixedFilterValuesArray.reset(filterCount);
    const float* filterValues = filterValuesArray.begin();
    SkConvolutionFilter1D::ConvolutionFixed* fixedFilterValues = fixedFilterValuesArray.begin();
    float invFilterSum = 1 / filterSum;
    for (int fixedI = 0; fixedI < filterCount; fixedI++) {
      int curFixed = SkConvolutionFilter1D::FloatToFixed(filterValues[fixedI] * invFilterSum);
      fixedSum += curFixed;
      fixedFilterValues[fixedI] = SkToS16(curFixed);
    }
    SkASSERT(fixedSum <= 0x7FFF);

    // The conversion to fixed point will leave some rounding errors, which
    // we add back in to avoid affecting the brightness of the image. We
    // arbitrarily add this to the center of the filter array (this won't always
    // be the center of the filter function since it could get clipped on the
    // edges, but it doesn't matter enough to worry about that case).
    int leftovers = SkConvolutionFilter1D::FloatToFixed(1) - fixedSum;
    fixedFilterValues[filterCount / 2] += leftovers;

    // Now it's ready to go.
    output->AddFilter(SkScalarFloorToInt(srcBegin), fixedFilterValues, filterCount);
  }

  if (convolveProcs.fApplySIMDPadding) {
      convolveProcs.fApplySIMDPadding(output);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool valid_for_resize(const SkPixmap& source, int dstW, int dstH) {
    // TODO: Seems like we shouldn't care about the swizzle of source, just that it's 8888
    return source.addr() && source.colorType() == kN32_SkColorType &&
           source.width() >= 1 && source.height() >= 1 && dstW >= 1 && dstH >= 1;
}

bool SkBitmapScaler::Resize(const SkPixmap& result, const SkPixmap& source, ResizeMethod method) {
    if (!valid_for_resize(source, result.width(), result.height())) {
        return false;
    }
    if (!result.addr() || result.colorType() != source.colorType()) {
        return false;
    }

    SkConvolutionProcs convolveProcs= { 0, nullptr, nullptr, nullptr, nullptr };
    PlatformConvolutionProcs(&convolveProcs);

    SkRect destSubset = SkRect::MakeIWH(result.width(), result.height());

    SkResizeFilter filter(method, source.width(), source.height(),
                          result.width(), result.height(), destSubset, convolveProcs);

    // Get a subset encompassing this touched area. We construct the
    // offsets and row strides such that it looks like a new bitmap, while
    // referring to the old data.
    const uint8_t* sourceSubset = reinterpret_cast<const uint8_t*>(source.addr());

    return BGRAConvolve2D(sourceSubset, static_cast<int>(source.rowBytes()),
                          !source.isOpaque(), filter.xFilter(), filter.yFilter(),
                          static_cast<int>(result.rowBytes()),
                          static_cast<unsigned char*>(result.writable_addr()),
                          convolveProcs, true);
}

bool SkBitmapScaler::Resize(SkBitmap* resultPtr, const SkPixmap& source, ResizeMethod method,
                            int destWidth, int destHeight, SkBitmap::Allocator* allocator) {
    // Preflight some of the checks, to avoid allocating the result if we don't need it.
    if (!valid_for_resize(source, destWidth, destHeight)) {
        return false;
    }

    SkBitmap result;
    result.setInfo(SkImageInfo::MakeN32(destWidth, destHeight, source.alphaType()));
    result.allocPixels(allocator, nullptr);

    SkPixmap resultPM;
    if (!result.peekPixels(&resultPM) || !Resize(resultPM, source, method)) {
        return false;
    }

    *resultPtr = result;
    resultPtr->lockPixels();
    SkASSERT(resultPtr->getPixels());
    return true;
}
