// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SK_CONVOLVER_H
#define SK_CONVOLVER_H

#include "SkSize.h"
#include "SkTypes.h"
#include "SkTArray.h"

// avoid confusion with Mac OS X's math library (Carbon)
#if defined(__APPLE__)
#undef FloatToConvolutionFixed
#undef ConvolutionFixedToFloat
#undef FloatToFixed
#undef FixedToFloat
#endif

// Represents a filter in one dimension. Each output pixel has one entry in this
// object for the filter values contributing to it. You build up the filter
// list by calling AddFilter for each output pixel (in order).
//
// We do 2-dimensional convolution by first convolving each row by one
// SkConvolutionFilter1D, then convolving each column by another one.
//
// Entries are stored in ConvolutionFixed point, shifted left by kShiftBits.
class SkConvolutionFilter1D {
public:
    typedef short ConvolutionFixed;

    // The number of bits that ConvolutionFixed point values are shifted by.
    enum { kShiftBits = 14 };

    SK_API SkConvolutionFilter1D();
    SK_API ~SkConvolutionFilter1D();

    // Convert between floating point and our ConvolutionFixed point representation.
    static ConvolutionFixed FloatToFixed(float f) {
        return static_cast<ConvolutionFixed>(f * (1 << kShiftBits));
    }
    static unsigned char FixedToChar(ConvolutionFixed x) {
        return static_cast<unsigned char>(x >> kShiftBits);
    }
    static float FixedToFloat(ConvolutionFixed x) {
        // The cast relies on ConvolutionFixed being a short, implying that on
        // the platforms we care about all (16) bits will fit into
        // the mantissa of a (32-bit) float.
        static_assert(sizeof(ConvolutionFixed) == 2, "ConvolutionFixed_type_should_fit_in_float_mantissa");
        float raw = static_cast<float>(x);
        return ldexpf(raw, -kShiftBits);
    }

    // Returns the maximum pixel span of a filter.
    int maxFilter() const { return fMaxFilter; }

    // Returns the number of filters in this filter. This is the dimension of the
    // output image.
    int numValues() const { return static_cast<int>(fFilters.count()); }

    // Appends the given list of scaling values for generating a given output
    // pixel. |filterOffset| is the distance from the edge of the image to where
    // the scaling factors start. The scaling factors apply to the source pixels
    // starting from this position, and going for the next |filterLength| pixels.
    //
    // You will probably want to make sure your input is normalized (that is,
    // all entries in |filterValuesg| sub to one) to prevent affecting the overall
    // brighness of the image.
    //
    // The filterLength must be > 0.
    //
    // This version will automatically convert your input to ConvolutionFixed point.
    SK_API void AddFilter(int filterOffset,
                          const float* filterValues,
                          int filterLength);

    // Same as the above version, but the input is already ConvolutionFixed point.
    void AddFilter(int filterOffset,
                   const ConvolutionFixed* filterValues,
                   int filterLength);

    // Retrieves a filter for the given |valueOffset|, a position in the output
    // image in the direction we're convolving. The offset and length of the
    // filter values are put into the corresponding out arguments (see AddFilter
    // above for what these mean), and a pointer to the first scaling factor is
    // returned. There will be |filterLength| values in this array.
    inline const ConvolutionFixed* FilterForValue(int valueOffset,
                                       int* filterOffset,
                                       int* filterLength) const {
        const FilterInstance& filter = fFilters[valueOffset];
        *filterOffset = filter.fOffset;
        *filterLength = filter.fTrimmedLength;
        if (filter.fTrimmedLength == 0) {
            return nullptr;
        }
        return &fFilterValues[filter.fDataLocation];
    }

  // Retrieves the filter for the offset 0, presumed to be the one and only.
  // The offset and length of the filter values are put into the corresponding
  // out arguments (see AddFilter). Note that |filterLegth| and
  // |specifiedFilterLength| may be different if leading/trailing zeros of the
  // original floating point form were clipped.
  // There will be |filterLength| values in the return array.
  // Returns nullptr if the filter is 0-length (for instance when all floating
  // point values passed to AddFilter were clipped to 0).
    SK_API const ConvolutionFixed* GetSingleFilter(int* specifiedFilterLength,
        int* filterOffset,
        int* filterLength) const;

    // Add another value to the fFilterValues array -- useful for
    // SIMD padding which happens outside of this class.

    void addFilterValue( ConvolutionFixed val ) {
        fFilterValues.push_back( val );
    }
private:
    struct FilterInstance {
        // Offset within filterValues for this instance of the filter.
        int fDataLocation;

        // Distance from the left of the filter to the center. IN PIXELS
        int fOffset;

        // Number of values in this filter instance.
        int fTrimmedLength;

        // Filter length as specified. Note that this may be different from
        // 'trimmed_length' if leading/trailing zeros of the original floating
        // point form were clipped differently on each tail.
        int fLength;
    };

    // Stores the information for each filter added to this class.
    SkTArray<FilterInstance> fFilters;

    // We store all the filter values in this flat list, indexed by
    // |FilterInstance.data_location| to avoid the mallocs required for storing
    // each one separately.
    SkTArray<ConvolutionFixed> fFilterValues;

    // The maximum size of any filter we've added.
    int fMaxFilter;
};

typedef void (*SkConvolveVertically_pointer)(
    const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
    int filterLength,
    unsigned char* const* sourceDataRows,
    int pixelWidth,
    unsigned char* outRow,
    bool hasAlpha);
typedef void (*SkConvolve4RowsHorizontally_pointer)(
    const unsigned char* srcData[4],
    const SkConvolutionFilter1D& filter,
    unsigned char* outRow[4],
    size_t outRowBytes);
typedef void (*SkConvolveHorizontally_pointer)(
    const unsigned char* srcData,
    const SkConvolutionFilter1D& filter,
    unsigned char* outRow,
    bool hasAlpha);
typedef void (*SkConvolveFilterPadding_pointer)(
    SkConvolutionFilter1D* filter);

struct SkConvolutionProcs {
  // This is how many extra pixels may be read by the
  // conolve*horizontally functions.
    int fExtraHorizontalReads;
    SkConvolveVertically_pointer fConvolveVertically;
    SkConvolve4RowsHorizontally_pointer fConvolve4RowsHorizontally;
    SkConvolveHorizontally_pointer fConvolveHorizontally;
    SkConvolveFilterPadding_pointer fApplySIMDPadding;
};



// Does a two-dimensional convolution on the given source image.
//
// It is assumed the source pixel offsets referenced in the input filters
// reference only valid pixels, so the source image size is not required. Each
// row of the source image starts |sourceByteRowStride| after the previous
// one (this allows you to have rows with some padding at the end).
//
// The result will be put into the given output buffer. The destination image
// size will be xfilter.numValues() * yfilter.numValues() pixels. It will be
// in rows of exactly xfilter.numValues() * 4 bytes.
//
// |sourceHasAlpha| is a hint that allows us to avoid doing computations on
// the alpha channel if the image is opaque. If you don't know, set this to
// true and it will work properly, but setting this to false will be a few
// percent faster if you know the image is opaque.
//
// The layout in memory is assumed to be 4-bytes per pixel in B-G-R-A order
// (this is ARGB when loaded into 32-bit words on a little-endian machine).
/**
 *  Returns false if it was unable to perform the convolution/rescale. in which case the output
 *  buffer is assumed to be undefined.
 */
SK_API bool BGRAConvolve2D(const unsigned char* sourceData,
    int sourceByteRowStride,
    bool sourceHasAlpha,
    const SkConvolutionFilter1D& xfilter,
    const SkConvolutionFilter1D& yfilter,
    int outputByteRowStride,
    unsigned char* output,
    const SkConvolutionProcs&,
    bool useSimdIfPossible);

#endif  // SK_CONVOLVER_H
