// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SkConvolver.h"
#include "SkSize.h"
#include "SkTypes.h"

namespace {

    // Converts the argument to an 8-bit unsigned value by clamping to the range
    // 0-255.
    inline unsigned char ClampTo8(int a) {
        if (static_cast<unsigned>(a) < 256) {
            return a;  // Avoid the extra check in the common case.
        }
        if (a < 0) {
            return 0;
        }
        return 255;
    }

    // Stores a list of rows in a circular buffer. The usage is you write into it
    // by calling AdvanceRow. It will keep track of which row in the buffer it
    // should use next, and the total number of rows added.
    class CircularRowBuffer {
    public:
        // The number of pixels in each row is given in |sourceRowPixelWidth|.
        // The maximum number of rows needed in the buffer is |maxYFilterSize|
        // (we only need to store enough rows for the biggest filter).
        //
        // We use the |firstInputRow| to compute the coordinates of all of the
        // following rows returned by Advance().
        CircularRowBuffer(int destRowPixelWidth, int maxYFilterSize,
                          int firstInputRow)
            : fRowByteWidth(destRowPixelWidth * 4),
              fNumRows(maxYFilterSize),
              fNextRow(0),
              fNextRowCoordinate(firstInputRow) {
            fBuffer.reset(fRowByteWidth * maxYFilterSize);
            fRowAddresses.reset(fNumRows);
        }

        // Moves to the next row in the buffer, returning a pointer to the beginning
        // of it.
        unsigned char* advanceRow() {
            unsigned char* row = &fBuffer[fNextRow * fRowByteWidth];
            fNextRowCoordinate++;

            // Set the pointer to the next row to use, wrapping around if necessary.
            fNextRow++;
            if (fNextRow == fNumRows) {
                fNextRow = 0;
            }
            return row;
        }

        // Returns a pointer to an "unrolled" array of rows. These rows will start
        // at the y coordinate placed into |*firstRowIndex| and will continue in
        // order for the maximum number of rows in this circular buffer.
        //
        // The |firstRowIndex_| may be negative. This means the circular buffer
        // starts before the top of the image (it hasn't been filled yet).
        unsigned char* const* GetRowAddresses(int* firstRowIndex) {
            // Example for a 4-element circular buffer holding coords 6-9.
            //   Row 0   Coord 8
            //   Row 1   Coord 9
            //   Row 2   Coord 6  <- fNextRow = 2, fNextRowCoordinate = 10.
            //   Row 3   Coord 7
            //
            // The "next" row is also the first (lowest) coordinate. This computation
            // may yield a negative value, but that's OK, the math will work out
            // since the user of this buffer will compute the offset relative
            // to the firstRowIndex and the negative rows will never be used.
            *firstRowIndex = fNextRowCoordinate - fNumRows;

            int curRow = fNextRow;
            for (int i = 0; i < fNumRows; i++) {
                fRowAddresses[i] = &fBuffer[curRow * fRowByteWidth];

                // Advance to the next row, wrapping if necessary.
                curRow++;
                if (curRow == fNumRows) {
                    curRow = 0;
                }
            }
            return &fRowAddresses[0];
        }

    private:
        // The buffer storing the rows. They are packed, each one fRowByteWidth.
        SkTArray<unsigned char> fBuffer;

        // Number of bytes per row in the |buffer|.
        int fRowByteWidth;

        // The number of rows available in the buffer.
        int fNumRows;

        // The next row index we should write into. This wraps around as the
        // circular buffer is used.
        int fNextRow;

        // The y coordinate of the |fNextRow|. This is incremented each time a
        // new row is appended and does not wrap.
        int fNextRowCoordinate;

        // Buffer used by GetRowAddresses().
        SkTArray<unsigned char*> fRowAddresses;
    };

// Convolves horizontally along a single row. The row data is given in
// |srcData| and continues for the numValues() of the filter.
template<bool hasAlpha>
    void ConvolveHorizontally(const unsigned char* srcData,
                              const SkConvolutionFilter1D& filter,
                              unsigned char* outRow) {
        // Loop over each pixel on this row in the output image.
        int numValues = filter.numValues();
        for (int outX = 0; outX < numValues; outX++) {
            // Get the filter that determines the current output pixel.
            int filterOffset, filterLength;
            const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
                filter.FilterForValue(outX, &filterOffset, &filterLength);

            // Compute the first pixel in this row that the filter affects. It will
            // touch |filterLength| pixels (4 bytes each) after this.
            const unsigned char* rowToFilter = &srcData[filterOffset * 4];

            // Apply the filter to the row to get the destination pixel in |accum|.
            int accum[4] = {0};
            for (int filterX = 0; filterX < filterLength; filterX++) {
                SkConvolutionFilter1D::ConvolutionFixed curFilter = filterValues[filterX];
                accum[0] += curFilter * rowToFilter[filterX * 4 + 0];
                accum[1] += curFilter * rowToFilter[filterX * 4 + 1];
                accum[2] += curFilter * rowToFilter[filterX * 4 + 2];
                if (hasAlpha) {
                    accum[3] += curFilter * rowToFilter[filterX * 4 + 3];
                }
            }

            // Bring this value back in range. All of the filter scaling factors
            // are in fixed point with kShiftBits bits of fractional part.
            accum[0] >>= SkConvolutionFilter1D::kShiftBits;
            accum[1] >>= SkConvolutionFilter1D::kShiftBits;
            accum[2] >>= SkConvolutionFilter1D::kShiftBits;
            if (hasAlpha) {
                accum[3] >>= SkConvolutionFilter1D::kShiftBits;
            }

            // Store the new pixel.
            outRow[outX * 4 + 0] = ClampTo8(accum[0]);
            outRow[outX * 4 + 1] = ClampTo8(accum[1]);
            outRow[outX * 4 + 2] = ClampTo8(accum[2]);
            if (hasAlpha) {
                outRow[outX * 4 + 3] = ClampTo8(accum[3]);
            }
        }
    }

    // There's a bug somewhere here with GCC autovectorization (-ftree-vectorize).  We originally
    // thought this was 32 bit only, but subsequent tests show that some 64 bit gcc compiles
    // suffer here too.
    //
    // Dropping to -O2 disables -ftree-vectorize.  GCC 4.6 needs noinline.  http://skbug.com/2575
    #if SK_HAS_ATTRIBUTE(optimize) && defined(SK_RELEASE)
        #define SK_MAYBE_DISABLE_VECTORIZATION __attribute__((optimize("O2"), noinline))
    #else
        #define SK_MAYBE_DISABLE_VECTORIZATION
    #endif

    SK_MAYBE_DISABLE_VECTORIZATION
    static void ConvolveHorizontallyAlpha(const unsigned char* srcData,
                                          const SkConvolutionFilter1D& filter,
                                          unsigned char* outRow) {
        return ConvolveHorizontally<true>(srcData, filter, outRow);
    }

    SK_MAYBE_DISABLE_VECTORIZATION
    static void ConvolveHorizontallyNoAlpha(const unsigned char* srcData,
                                            const SkConvolutionFilter1D& filter,
                                            unsigned char* outRow) {
        return ConvolveHorizontally<false>(srcData, filter, outRow);
    }

    #undef SK_MAYBE_DISABLE_VECTORIZATION


// Does vertical convolution to produce one output row. The filter values and
// length are given in the first two parameters. These are applied to each
// of the rows pointed to in the |sourceDataRows| array, with each row
// being |pixelWidth| wide.
//
// The output must have room for |pixelWidth * 4| bytes.
template<bool hasAlpha>
    void ConvolveVertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow) {
        // We go through each column in the output and do a vertical convolution,
        // generating one output pixel each time.
        for (int outX = 0; outX < pixelWidth; outX++) {
            // Compute the number of bytes over in each row that the current column
            // we're convolving starts at. The pixel will cover the next 4 bytes.
            int byteOffset = outX * 4;

            // Apply the filter to one column of pixels.
            int accum[4] = {0};
            for (int filterY = 0; filterY < filterLength; filterY++) {
                SkConvolutionFilter1D::ConvolutionFixed curFilter = filterValues[filterY];
                accum[0] += curFilter * sourceDataRows[filterY][byteOffset + 0];
                accum[1] += curFilter * sourceDataRows[filterY][byteOffset + 1];
                accum[2] += curFilter * sourceDataRows[filterY][byteOffset + 2];
                if (hasAlpha) {
                    accum[3] += curFilter * sourceDataRows[filterY][byteOffset + 3];
                }
            }

            // Bring this value back in range. All of the filter scaling factors
            // are in fixed point with kShiftBits bits of precision.
            accum[0] >>= SkConvolutionFilter1D::kShiftBits;
            accum[1] >>= SkConvolutionFilter1D::kShiftBits;
            accum[2] >>= SkConvolutionFilter1D::kShiftBits;
            if (hasAlpha) {
                accum[3] >>= SkConvolutionFilter1D::kShiftBits;
            }

            // Store the new pixel.
            outRow[byteOffset + 0] = ClampTo8(accum[0]);
            outRow[byteOffset + 1] = ClampTo8(accum[1]);
            outRow[byteOffset + 2] = ClampTo8(accum[2]);
            if (hasAlpha) {
                unsigned char alpha = ClampTo8(accum[3]);

                // Make sure the alpha channel doesn't come out smaller than any of the
                // color channels. We use premultipled alpha channels, so this should
                // never happen, but rounding errors will cause this from time to time.
                // These "impossible" colors will cause overflows (and hence random pixel
                // values) when the resulting bitmap is drawn to the screen.
                //
                // We only need to do this when generating the final output row (here).
                int maxColorChannel = SkTMax(outRow[byteOffset + 0],
                                               SkTMax(outRow[byteOffset + 1],
                                                      outRow[byteOffset + 2]));
                if (alpha < maxColorChannel) {
                    outRow[byteOffset + 3] = maxColorChannel;
                } else {
                    outRow[byteOffset + 3] = alpha;
                }
            } else {
                // No alpha channel, the image is opaque.
                outRow[byteOffset + 3] = 0xff;
            }
        }
    }

    void ConvolveVertically(const SkConvolutionFilter1D::ConvolutionFixed* filterValues,
                            int filterLength,
                            unsigned char* const* sourceDataRows,
                            int pixelWidth,
                            unsigned char* outRow,
                            bool sourceHasAlpha) {
        if (sourceHasAlpha) {
            ConvolveVertically<true>(filterValues, filterLength,
                                     sourceDataRows, pixelWidth,
                                     outRow);
        } else {
            ConvolveVertically<false>(filterValues, filterLength,
                                      sourceDataRows, pixelWidth,
                                      outRow);
        }
    }

}  // namespace

// SkConvolutionFilter1D ---------------------------------------------------------

SkConvolutionFilter1D::SkConvolutionFilter1D()
: fMaxFilter(0) {
}

SkConvolutionFilter1D::~SkConvolutionFilter1D() {
}

void SkConvolutionFilter1D::AddFilter(int filterOffset,
                                      const float* filterValues,
                                      int filterLength) {
    SkASSERT(filterLength > 0);

    SkTArray<ConvolutionFixed> fixedValues;
    fixedValues.reset(filterLength);

    for (int i = 0; i < filterLength; ++i) {
        fixedValues.push_back(FloatToFixed(filterValues[i]));
    }

    AddFilter(filterOffset, &fixedValues[0], filterLength);
}

void SkConvolutionFilter1D::AddFilter(int filterOffset,
                                      const ConvolutionFixed* filterValues,
                                      int filterLength) {
    // It is common for leading/trailing filter values to be zeros. In such
    // cases it is beneficial to only store the central factors.
    // For a scaling to 1/4th in each dimension using a Lanczos-2 filter on
    // a 1080p image this optimization gives a ~10% speed improvement.
    int filterSize = filterLength;
    int firstNonZero = 0;
    while (firstNonZero < filterLength && filterValues[firstNonZero] == 0) {
        firstNonZero++;
    }

    if (firstNonZero < filterLength) {
        // Here we have at least one non-zero factor.
        int lastNonZero = filterLength - 1;
        while (lastNonZero >= 0 && filterValues[lastNonZero] == 0) {
            lastNonZero--;
        }

        filterOffset += firstNonZero;
        filterLength = lastNonZero + 1 - firstNonZero;
        SkASSERT(filterLength > 0);

        for (int i = firstNonZero; i <= lastNonZero; i++) {
            fFilterValues.push_back(filterValues[i]);
        }
    } else {
        // Here all the factors were zeroes.
        filterLength = 0;
    }

    FilterInstance instance;

    // We pushed filterLength elements onto fFilterValues
    instance.fDataLocation = (static_cast<int>(fFilterValues.count()) -
                                               filterLength);
    instance.fOffset = filterOffset;
    instance.fTrimmedLength = filterLength;
    instance.fLength = filterSize;
    fFilters.push_back(instance);

    fMaxFilter = SkTMax(fMaxFilter, filterLength);
}

const SkConvolutionFilter1D::ConvolutionFixed* SkConvolutionFilter1D::GetSingleFilter(
                                        int* specifiedFilterlength,
                                        int* filterOffset,
                                        int* filterLength) const {
    const FilterInstance& filter = fFilters[0];
    *filterOffset = filter.fOffset;
    *filterLength = filter.fTrimmedLength;
    *specifiedFilterlength = filter.fLength;
    if (filter.fTrimmedLength == 0) {
        return NULL;
    }

    return &fFilterValues[filter.fDataLocation];
}

void BGRAConvolve2D(const unsigned char* sourceData,
                    int sourceByteRowStride,
                    bool sourceHasAlpha,
                    const SkConvolutionFilter1D& filterX,
                    const SkConvolutionFilter1D& filterY,
                    int outputByteRowStride,
                    unsigned char* output,
                    const SkConvolutionProcs& convolveProcs,
                    bool useSimdIfPossible) {

    int maxYFilterSize = filterY.maxFilter();

    // The next row in the input that we will generate a horizontally
    // convolved row for. If the filter doesn't start at the beginning of the
    // image (this is the case when we are only resizing a subset), then we
    // don't want to generate any output rows before that. Compute the starting
    // row for convolution as the first pixel for the first vertical filter.
    int filterOffset, filterLength;
    const SkConvolutionFilter1D::ConvolutionFixed* filterValues =
        filterY.FilterForValue(0, &filterOffset, &filterLength);
    int nextXRow = filterOffset;

    // We loop over each row in the input doing a horizontal convolution. This
    // will result in a horizontally convolved image. We write the results into
    // a circular buffer of convolved rows and do vertical convolution as rows
    // are available. This prevents us from having to store the entire
    // intermediate image and helps cache coherency.
    // We will need four extra rows to allow horizontal convolution could be done
    // simultaneously. We also pad each row in row buffer to be aligned-up to
    // 16 bytes.
    // TODO(jiesun): We do not use aligned load from row buffer in vertical
    // convolution pass yet. Somehow Windows does not like it.
    int rowBufferWidth = (filterX.numValues() + 15) & ~0xF;
    int rowBufferHeight = maxYFilterSize +
                          (convolveProcs.fConvolve4RowsHorizontally ? 4 : 0);
    CircularRowBuffer rowBuffer(rowBufferWidth,
                                rowBufferHeight,
                                filterOffset);

    // Loop over every possible output row, processing just enough horizontal
    // convolutions to run each subsequent vertical convolution.
    SkASSERT(outputByteRowStride >= filterX.numValues() * 4);
    int numOutputRows = filterY.numValues();

    // We need to check which is the last line to convolve before we advance 4
    // lines in one iteration.
    int lastFilterOffset, lastFilterLength;

    // SSE2 can access up to 3 extra pixels past the end of the
    // buffer. At the bottom of the image, we have to be careful
    // not to access data past the end of the buffer. Normally
    // we fall back to the C++ implementation for the last row.
    // If the last row is less than 3 pixels wide, we may have to fall
    // back to the C++ version for more rows. Compute how many
    // rows we need to avoid the SSE implementation for here.
    filterX.FilterForValue(filterX.numValues() - 1, &lastFilterOffset,
                           &lastFilterLength);
    int avoidSimdRows = 1 + convolveProcs.fExtraHorizontalReads /
        (lastFilterOffset + lastFilterLength);

    filterY.FilterForValue(numOutputRows - 1, &lastFilterOffset,
                           &lastFilterLength);

    for (int outY = 0; outY < numOutputRows; outY++) {
        filterValues = filterY.FilterForValue(outY,
                                              &filterOffset, &filterLength);

        // Generate output rows until we have enough to run the current filter.
        while (nextXRow < filterOffset + filterLength) {
            if (convolveProcs.fConvolve4RowsHorizontally &&
                nextXRow + 3 < lastFilterOffset + lastFilterLength -
                avoidSimdRows) {
                const unsigned char* src[4];
                unsigned char* outRow[4];
                for (int i = 0; i < 4; ++i) {
                    src[i] = &sourceData[(uint64_t)(nextXRow + i) * sourceByteRowStride];
                    outRow[i] = rowBuffer.advanceRow();
                }
                convolveProcs.fConvolve4RowsHorizontally(src, filterX, outRow, 4*rowBufferWidth);
                nextXRow += 4;
            } else {
                // Check if we need to avoid SSE2 for this row.
                if (convolveProcs.fConvolveHorizontally &&
                    nextXRow < lastFilterOffset + lastFilterLength -
                    avoidSimdRows) {
                    convolveProcs.fConvolveHorizontally(
                        &sourceData[(uint64_t)nextXRow * sourceByteRowStride],
                        filterX, rowBuffer.advanceRow(), sourceHasAlpha);
                } else {
                    if (sourceHasAlpha) {
                        ConvolveHorizontallyAlpha(
                            &sourceData[(uint64_t)nextXRow * sourceByteRowStride],
                            filterX, rowBuffer.advanceRow());
                    } else {
                        ConvolveHorizontallyNoAlpha(
                            &sourceData[(uint64_t)nextXRow * sourceByteRowStride],
                            filterX, rowBuffer.advanceRow());
                    }
                }
                nextXRow++;
            }
        }

        // Compute where in the output image this row of final data will go.
        unsigned char* curOutputRow = &output[(uint64_t)outY * outputByteRowStride];

        // Get the list of rows that the circular buffer has, in order.
        int firstRowInCircularBuffer;
        unsigned char* const* rowsToConvolve =
            rowBuffer.GetRowAddresses(&firstRowInCircularBuffer);

        // Now compute the start of the subset of those rows that the filter
        // needs.
        unsigned char* const* firstRowForFilter =
            &rowsToConvolve[filterOffset - firstRowInCircularBuffer];

        if (convolveProcs.fConvolveVertically) {
            convolveProcs.fConvolveVertically(filterValues, filterLength,
                                               firstRowForFilter,
                                               filterX.numValues(), curOutputRow,
                                               sourceHasAlpha);
        } else {
            ConvolveVertically(filterValues, filterLength,
                               firstRowForFilter,
                               filterX.numValues(), curOutputRow,
                               sourceHasAlpha);
        }
    }
}
