// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SkConvolver.h"
#include "SkOpts.h"
#include "SkTArray.h"

namespace {
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

}  // namespace

// SkConvolutionFilter1D ---------------------------------------------------------

SkConvolutionFilter1D::SkConvolutionFilter1D()
: fMaxFilter(0) {
}

SkConvolutionFilter1D::~SkConvolutionFilter1D() {
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

        fFilterValues.append(filterLength, &filterValues[firstNonZero]);
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
    fFilters.push(instance);

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
        return nullptr;
    }

    return &fFilterValues[filter.fDataLocation];
}

bool BGRAConvolve2D(const unsigned char* sourceData,
                    int sourceByteRowStride,
                    bool sourceHasAlpha,
                    const SkConvolutionFilter1D& filterX,
                    const SkConvolutionFilter1D& filterY,
                    int outputByteRowStride,
                    unsigned char* output) {

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
    // 32 bytes.
    // TODO(jiesun): We do not use aligned load from row buffer in vertical
    // convolution pass yet. Somehow Windows does not like it.
    int rowBufferWidth = (filterX.numValues() + 31) & ~0x1F;
    int rowBufferHeight = maxYFilterSize +
                          (SkOpts::convolve_4_rows_horizontally != nullptr ? 4 : 0);

    // check for too-big allocation requests : crbug.com/528628
    {
        int64_t size = sk_64_mul(rowBufferWidth, rowBufferHeight);
        // need some limit, to avoid over-committing success from malloc, but then
        // crashing when we try to actually use the memory.
        // 100meg seems big enough to allow "normal" zoom factors and image sizes through
        // while avoiding the crash seen by the bug (crbug.com/528628)
        if (size > 100 * 1024 * 1024) {
//            SkDebugf("BGRAConvolve2D: tmp allocation [%lld] too big\n", size);
            return false;
        }
    }

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
    filterY.FilterForValue(numOutputRows - 1, &lastFilterOffset,
                           &lastFilterLength);

    for (int outY = 0; outY < numOutputRows; outY++) {
        filterValues = filterY.FilterForValue(outY,
                                              &filterOffset, &filterLength);

        // Generate output rows until we have enough to run the current filter.
        while (nextXRow < filterOffset + filterLength) {
            if (SkOpts::convolve_4_rows_horizontally != nullptr &&
                nextXRow + 3 < lastFilterOffset + lastFilterLength) {
                const unsigned char* src[4];
                unsigned char* outRow[4];
                for (int i = 0; i < 4; ++i) {
                    src[i] = &sourceData[(uint64_t)(nextXRow + i) * sourceByteRowStride];
                    outRow[i] = rowBuffer.advanceRow();
                }
                SkOpts::convolve_4_rows_horizontally(src, filterX, outRow, 4*rowBufferWidth);
                nextXRow += 4;
            } else {
                SkOpts::convolve_horizontally(
                        &sourceData[(uint64_t)nextXRow * sourceByteRowStride],
                        filterX, rowBuffer.advanceRow(), sourceHasAlpha);
                nextXRow++;
            }
        }

        // Compute where in the output image this row of final data will go.
        unsigned char* curOutputRow = &output[(uint64_t)outY * outputByteRowStride];

        // Get the list of rows that the circular buffer has, in order.
        int firstRowInCircularBuffer;
        unsigned char* const* rowsToConvolve =
            rowBuffer.GetRowAddresses(&firstRowInCircularBuffer);

        // Now compute the start of the subset of those rows that the filter needs.
        unsigned char* const* firstRowForFilter =
            &rowsToConvolve[filterOffset - firstRowInCircularBuffer];

        SkOpts::convolve_vertically(filterValues, filterLength,
                                    firstRowForFilter,
                                    filterX.numValues(), curOutputRow,
                                    sourceHasAlpha);
    }
    return true;
}
