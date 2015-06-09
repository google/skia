/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_Blitter_DEFINED
#define SkTextureCompressor_Blitter_DEFINED

#include "SkTypes.h"
#include "SkBlitter.h"

namespace SkTextureCompressor {

// Ostensibly, SkBlitter::BlitRect is supposed to set a rect of pixels to full
// alpha. This becomes problematic when using compressed texture blitters, since
// the rect rarely falls along block boundaries. The proper way to handle this is
// to update the compressed encoding of a block by resetting the proper parameters
// (and even recompressing the block) where a rect falls inbetween block boundaries.
// PEDANTIC_BLIT_RECT attempts to do this by requiring the struct passed to
// SkTCompressedAlphaBlitter to implement an UpdateBlock function call.
//
// However, the way that BlitRect gets used almost exclusively is to bracket inverse
// fills for paths. In other words, the top few rows and bottom few rows of a path
// that's getting inverse filled are called using blitRect. The rest are called using
// the standard blitAntiH. As a result, we can just call  blitAntiH with a faux RLE
// of full alpha values, and then check in our flush() call that we don't run off the
// edge of the buffer. This is why we do not need this flag to be turned on.
//
// NOTE: This code is unfinished, but is inteded as a starting point if an when
// bugs are introduced from the existing code.
#define PEDANTIC_BLIT_RECT 0

// This class implements a blitter that blits directly into a buffer that will
// be used as an compressed alpha texture. We compute this buffer by
// buffering scan lines and then outputting them all at once. The number of
// scan lines buffered is controlled by kBlockSize
//
// The CompressorType is a struct with a bunch of static methods that provides
// the specialized compression functionality of the blitter. A complete CompressorType
// will implement the following static functions;
//
// struct CompressorType {
//     // The function used to compress an A8 block. The layout of the
//     // block is also expected to be in column-major order.
//     static void CompressA8Vertical(uint8_t* dst, const uint8_t block[]);
//
//     // The function used to compress an A8 block. The layout of the
//     // block is also expected to be in row-major order.
//     static void CompressA8Horizontal(uint8_t* dst, const uint8_t* src, int srcRowBytes);
//
#if PEDANTIC_BLIT_RECT
//     // The function used to update an already compressed block. This will
//     // most likely be implementation dependent. The mask variable will have
//     // 0xFF in positions where the block should be updated and 0 in positions
//     // where it shouldn't. src contains an uncompressed buffer of pixels.
//     static void UpdateBlock(uint8_t* dst, const uint8_t* src, int srcRowBytes, 
//                             const uint8_t* mask);
#endif
// };
template<int BlockDim, int EncodedBlockSize, typename CompressorType>
class SkTCompressedAlphaBlitter : public SkBlitter {
public:
    SkTCompressedAlphaBlitter(int width, int height, void *compressedBuffer)
        // 0x7FFE is one minus the largest positive 16-bit int. We use it for
        // debugging to make sure that we're properly setting the nextX distance
        // in flushRuns(). 
#ifdef SK_DEBUG
        : fCalledOnceWithNonzeroY(false)
        , fBlitMaskCalled(false),
#else
        :
#endif
        kLongestRun(0x7FFE), kZeroAlpha(0)
        , fNextRun(0)
        , fWidth(width)
        , fHeight(height)
        , fBuffer(compressedBuffer)
        {
            SkASSERT((width % BlockDim) == 0);
            SkASSERT((height % BlockDim) == 0);
        }

    virtual ~SkTCompressedAlphaBlitter() { this->flushRuns(); }

    // Blit a horizontal run of one or more pixels.
    void blitH(int x, int y, int width) override {
        // This function is intended to be called from any standard RGB
        // buffer, so we should never encounter it. However, if some code
        // path does end up here, then this needs to be investigated.
        SkFAIL("Not implemented!");
    }

    // Blit a horizontal run of antialiased pixels; runs[] is a *sparse*
    // zero-terminated run-length encoding of spans of constant alpha values.
    void blitAntiH(int x, int y,
                   const SkAlpha antialias[],
                   const int16_t runs[]) override {
        SkASSERT(0 == x);

        // Make sure that the new row to blit is either the first
        // row that we're blitting, or it's exactly the next scan row
        // since the last row that we blit. This is to ensure that when
        // we go to flush the runs, that they are all the same four
        // runs.
        if (fNextRun > 0 &&
            ((x != fBufferedRuns[fNextRun-1].fX) ||
             (y-1 != fBufferedRuns[fNextRun-1].fY))) {
            this->flushRuns();
        }

        // Align the rows to a block boundary. If we receive rows that
        // are not on a block boundary, then fill in the preceding runs
        // with zeros. We do this by producing a single RLE that says
        // that we have 0x7FFE pixels of zero (0x7FFE = 32766).
        const int row = BlockDim * (y / BlockDim);
        while ((row + fNextRun) < y) {
            fBufferedRuns[fNextRun].fAlphas = &kZeroAlpha;
            fBufferedRuns[fNextRun].fRuns = &kLongestRun;
            fBufferedRuns[fNextRun].fX = 0;
            fBufferedRuns[fNextRun].fY = row + fNextRun;
            ++fNextRun;
        }

        // Make sure that our assumptions aren't violated...
        SkASSERT(fNextRun == (y % BlockDim));
        SkASSERT(fNextRun == 0 || fBufferedRuns[fNextRun - 1].fY < y);

        // Set the values of the next run
        fBufferedRuns[fNextRun].fAlphas = antialias;
        fBufferedRuns[fNextRun].fRuns = runs;
        fBufferedRuns[fNextRun].fX = x;
        fBufferedRuns[fNextRun].fY = y;

        // If we've output a block of scanlines in a row that don't violate our
        // assumptions, then it's time to flush them...
        if (BlockDim == ++fNextRun) {
            this->flushRuns();
        }
    }
    
    // Blit a vertical run of pixels with a constant alpha value.
    void blitV(int x, int y, int height, SkAlpha alpha) override {
        // This function is currently not implemented. It is not explicitly
        // required by the contract, but if at some time a code path runs into
        // this function (which is entirely possible), it needs to be implemented.
        //
        // TODO (krajcevski):
        // This function will be most easily implemented in one of two ways:
        // 1. Buffer each vertical column value and then construct a list
        //    of alpha values and output all of the blocks at once. This only
        //    requires a write to the compressed buffer
        // 2. Replace the indices of each block with the proper indices based
        //    on the alpha value. This requires a read and write of the compressed
        //    buffer, but much less overhead.
        SkFAIL("Not implemented!");
    }

    // Blit a solid rectangle one or more pixels wide. It's assumed that blitRect
    // is called as a way to bracket blitAntiH where above and below the path the
    // called path just needs a solid rectangle to fill in the mask.
#ifdef SK_DEBUG
    bool fCalledOnceWithNonzeroY;
#endif
    void blitRect(int x, int y, int width, int height) override {

        // Assumptions:
        SkASSERT(0 == x);
        SkASSERT(width <= fWidth);

        // Make sure that we're only ever bracketing calls to blitAntiH.
        SkASSERT((0 == y) || (!fCalledOnceWithNonzeroY && (fCalledOnceWithNonzeroY = true)));
        
#if !(PEDANTIC_BLIT_RECT)
        for (int i = 0; i < height; ++i) {
            const SkAlpha kFullAlpha = 0xFF;
            this->blitAntiH(x, y+i, &kFullAlpha, &kLongestRun);
        }
#else
        const int startBlockX = (x / BlockDim) * BlockDim;
        const int startBlockY = (y / BlockDim) * BlockDim;

        const int endBlockX = ((x + width) / BlockDim) * BlockDim;
        const int endBlockY = ((y + height) / BlockDim) * BlockDim;

        // If start and end are the same, then we only need to update a single block...
        if (startBlockY == endBlockY && startBlockX == endBlockX) {
            uint8_t mask[BlockDim*BlockDim];
            memset(mask, 0, sizeof(mask));

            const int xoff = x - startBlockX;
            SkASSERT((xoff + width) <= BlockDim);

            const int yoff = y - startBlockY;
            SkASSERT((yoff + height) <= BlockDim);
            
            for (int j = 0; j < height; ++j) {
                memset(mask + (j + yoff)*BlockDim + xoff, 0xFF, width);
            }

            uint8_t* dst = this->getBlock(startBlockX, startBlockY);
            CompressorType::UpdateBlock(dst, mask, BlockDim, mask);

        // If start and end are the same in the y dimension, then we can freely update an
        // entire row of blocks...
        } else if (startBlockY == endBlockY) {

            this->updateBlockRow(x, y, width, height, startBlockY, startBlockX, endBlockX);

        // Similarly, if the start and end are in the same column, then we can just update
        // an entire column of blocks...
        } else if (startBlockX == endBlockX) {

            this->updateBlockCol(x, y, width, height, startBlockX, startBlockY, endBlockY);

        // Otherwise, the rect spans a non-trivial region of blocks, and we have to construct
        // a kind of 9-patch to update each of the pieces of the rect. The top and bottom
        // rows are updated using updateBlockRow, and the left and right columns are updated
        // using updateBlockColumn. Anything in the middle is simply memset to an opaque block
        // encoding.
        } else {

            const int innerStartBlockX = startBlockX + BlockDim;
            const int innerStartBlockY = startBlockY + BlockDim;

            // Blit top row
            const int topRowHeight = innerStartBlockY - y;
            this->updateBlockRow(x, y, width, topRowHeight, startBlockY,
                                 startBlockX, endBlockX);

            // Advance y
            y += topRowHeight;
            height -= topRowHeight;

            // Blit middle
            if (endBlockY > innerStartBlockY) {

                // Update left row
                this->updateBlockCol(x, y, innerStartBlockX - x, endBlockY, startBlockY,
                                     startBlockX, innerStartBlockX);

                // Update the middle with an opaque encoding...
                uint8_t mask[BlockDim*BlockDim];
                memset(mask, 0xFF, sizeof(mask));

                uint8_t opaqueEncoding[EncodedBlockSize];
                CompressorType::CompressA8Horizontal(opaqueEncoding, mask, BlockDim);

                for (int j = innerStartBlockY; j < endBlockY; j += BlockDim) {
                    uint8_t* opaqueDst = this->getBlock(innerStartBlockX, j);
                    for (int i = innerStartBlockX; i < endBlockX; i += BlockDim) {
                        memcpy(opaqueDst, opaqueEncoding, EncodedBlockSize);
                        opaqueDst += EncodedBlockSize;
                    }
                }

                // If we need to update the right column, do that too
                if (x + width > endBlockX) {
                    this->updateBlockCol(endBlockX, y, x + width - endBlockX, endBlockY,
                                         endBlockX, innerStartBlockY, endBlockY);
                }

                // Advance y
                height = y + height - endBlockY;
                y = endBlockY;
            }

            // If we need to update the last row, then do that, too.
            if (height > 0) {
                this->updateBlockRow(x, y, width, height, endBlockY,
                                     startBlockX, endBlockX);
            }
        }
#endif
    }

    // Blit a rectangle with one alpha-blended column on the left,
    // width (zero or more) opaque pixels, and one alpha-blended column
    // on the right. The result will always be at least two pixels wide.
    void blitAntiRect(int x, int y, int width, int height,
                      SkAlpha leftAlpha, SkAlpha rightAlpha) override {
        // This function is currently not implemented. It is not explicitly
        // required by the contract, but if at some time a code path runs into
        // this function (which is entirely possible), it needs to be implemented.
        //
        // TODO (krajcevski):
        // This function will be most easily implemented as follows:
        // 1. If width/height are smaller than a block, then update the
        //    indices of the affected blocks.
        // 2. If width/height are larger than a block, then construct a 9-patch
        //    of block encodings that represent the rectangle, and write them
        //    to the compressed buffer as necessary. Whether or not the blocks
        //    are overwritten by zeros or just their indices are updated is up
        //    to debate.
        SkFAIL("Not implemented!");
    }

    // Blit a pattern of pixels defined by a rectangle-clipped mask; We make an
    // assumption here that if this function gets called, then it will replace all
    // of the compressed texture blocks that it touches. Hence, two separate calls
    // to blitMask that have clips next to one another will cause artifacts. Most
    // of the time, however, this function gets called because constructing the mask
    // was faster than constructing the RLE for blitAntiH, and this function will
    // only be called once.
#ifdef SK_DEBUG
    bool fBlitMaskCalled;
#endif
    void blitMask(const SkMask& mask, const SkIRect& clip) override {

        // Assumptions:
        SkASSERT(!fBlitMaskCalled);
        SkDEBUGCODE(fBlitMaskCalled = true);
        SkASSERT(SkMask::kA8_Format == mask.fFormat);
        SkASSERT(mask.fBounds.contains(clip));

        // Start from largest block boundary less than the clip boundaries.
        const int startI = BlockDim * (clip.left() / BlockDim);
        const int startJ = BlockDim * (clip.top() / BlockDim);

        for (int j = startJ; j < clip.bottom(); j += BlockDim) {

            // Get the destination for this block row
            uint8_t* dst = this->getBlock(startI, j);
            for (int i = startI; i < clip.right(); i += BlockDim) {

                // At this point, the block should intersect the clip.
                SkASSERT(SkIRect::IntersectsNoEmptyCheck(
                             SkIRect::MakeXYWH(i, j, BlockDim, BlockDim), clip));

                // Do we need to pad it?
                if (i < clip.left() || j < clip.top() ||
                    i + BlockDim > clip.right() || j + BlockDim > clip.bottom()) {

                    uint8_t block[BlockDim*BlockDim];
                    memset(block, 0, sizeof(block));

                    const int startX = SkMax32(i, clip.left());
                    const int startY = SkMax32(j, clip.top());

                    const int endX = SkMin32(i + BlockDim, clip.right());
                    const int endY = SkMin32(j + BlockDim, clip.bottom());

                    for (int y = startY; y < endY; ++y) {
                        const int col = startX - i;
                        const int row = y - j;
                        const int valsWide = endX - startX;
                        SkASSERT(valsWide <= BlockDim);
                        SkASSERT(0 <= col && col < BlockDim);
                        SkASSERT(0 <= row && row < BlockDim);
                        memcpy(block + row*BlockDim + col,
                               mask.getAddr8(startX, j + row), valsWide);
                    }

                    CompressorType::CompressA8Horizontal(dst, block, BlockDim);
                } else {
                    // Otherwise, just compress it.
                    uint8_t*const src = mask.getAddr8(i, j);
                    const uint32_t rb = mask.fRowBytes;
                    CompressorType::CompressA8Horizontal(dst, src, rb);
                }

                dst += EncodedBlockSize;
            }
        }
    }

    // If the blitter just sets a single value for each pixel, return the
    // bitmap it draws into, and assign value. If not, return NULL and ignore
    // the value parameter.
    const SkPixmap* justAnOpaqueColor(uint32_t* value) override {
        return NULL;
    }

    /**
     * Compressed texture blitters only really work correctly if they get
     * BlockDim rows at a time. That being said, this blitter tries it's best
     * to preserve semantics if blitAntiH doesn't get called in too many
     * weird ways...
     */
    int requestRowsPreserved() const override { return BlockDim; }

private:
    static const int kPixelsPerBlock = BlockDim * BlockDim;

    // The longest possible run of pixels that this blitter will receive.
    // This is initialized in the constructor to 0x7FFE, which is one less
    // than the largest positive 16-bit integer. We make sure that it's one
    // less for debugging purposes. We also don't make this variable static
    // in order to make sure that we can construct a valid pointer to it.
    const int16_t kLongestRun;

    // Usually used in conjunction with kLongestRun. This is initialized to
    // zero.
    const SkAlpha kZeroAlpha;

    // This is the information that we buffer whenever we're asked to blit
    // a row with this blitter.
    struct BufferedRun {
        const SkAlpha* fAlphas;
        const int16_t* fRuns;
        int fX, fY;
    } fBufferedRuns[BlockDim];

    // The next row [0, BlockDim) that we need to blit.
    int fNextRun;

    // The width and height of the image that we're blitting
    const int fWidth;
    const int fHeight;

    // The compressed buffer that we're blitting into. It is assumed that the buffer
    // is large enough to store a compressed image of size fWidth*fHeight.
    void* const fBuffer;

    // Various utility functions
    int blocksWide() const { return fWidth / BlockDim; }
    int blocksTall() const { return fHeight / BlockDim; }
    int totalBlocks() const { return (fWidth * fHeight) / kPixelsPerBlock; }

    // Returns the block index for the block containing pixel (x, y). Block
    // indices start at zero and proceed in raster order.
    int getBlockOffset(int x, int y) const {
        SkASSERT(x < fWidth);
        SkASSERT(y < fHeight);
        const int blockCol = x / BlockDim;
        const int blockRow = y / BlockDim;
        return blockRow * this->blocksWide() + blockCol;
    }

    // Returns a pointer to the block containing pixel (x, y)
    uint8_t *getBlock(int x, int y) const {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(fBuffer);
        return ptr + EncodedBlockSize*this->getBlockOffset(x, y);
    }

    // Updates the block whose columns are stored in block. curAlphai is expected
    // to store the alpha values that will be placed within each of the columns in
    // the range [col, col+colsLeft).
    typedef uint32_t Column[BlockDim/4];
    typedef uint32_t Block[BlockDim][BlockDim/4];
    inline void updateBlockColumns(Block block, const int col,
                                   const int colsLeft, const Column curAlphai) {
        SkASSERT(block);
        SkASSERT(col + colsLeft <= BlockDim);

        for (int i = col; i < (col + colsLeft); ++i) {
            memcpy(block[i], curAlphai, sizeof(Column));
        }
    }

    // The following function writes the buffered runs to compressed blocks.
    // If fNextRun < BlockDim, then we fill the runs that we haven't buffered with
    // the constant zero buffer.
    void flushRuns() {
        // If we don't have any runs, then just return.
        if (0 == fNextRun) {
            return;
        }

#ifndef NDEBUG
        // Make sure that if we have any runs, they all match
        for (int i = 1; i < fNextRun; ++i) {
            SkASSERT(fBufferedRuns[i].fY == fBufferedRuns[i-1].fY + 1);
            SkASSERT(fBufferedRuns[i].fX == fBufferedRuns[i-1].fX);
        }
#endif

        // If we don't have as many runs as we have rows, fill in the remaining
        // runs with constant zeros.
        for (int i = fNextRun; i < BlockDim; ++i) {
            fBufferedRuns[i].fY = fBufferedRuns[0].fY + i;
            fBufferedRuns[i].fX = fBufferedRuns[0].fX;
            fBufferedRuns[i].fAlphas = &kZeroAlpha;
            fBufferedRuns[i].fRuns = &kLongestRun;
        }

        // Make sure that our assumptions aren't violated.
        SkASSERT(fNextRun > 0 && fNextRun <= BlockDim);
        SkASSERT((fBufferedRuns[0].fY % BlockDim) == 0);

        // The following logic walks BlockDim rows at a time and outputs compressed
        // blocks to the buffer passed into the constructor.
        // We do the following:
        //
        //      c1 c2 c3 c4
        // -----------------------------------------------------------------------
        // ... |  |  |  |  |  ----> fBufferedRuns[0]
        // -----------------------------------------------------------------------
        // ... |  |  |  |  |  ----> fBufferedRuns[1]
        // -----------------------------------------------------------------------
        // ... |  |  |  |  |  ----> fBufferedRuns[2]
        // -----------------------------------------------------------------------
        // ... |  |  |  |  |  ----> fBufferedRuns[3]
        // -----------------------------------------------------------------------
        // 
        // curX -- the macro X value that we've gotten to.
        // c[BlockDim] -- the buffers that represent the columns of the current block
        //                  that we're operating on
        // curAlphaColumn -- buffer containing the column of alpha values from fBufferedRuns.
        // nextX -- for each run, the next point at which we need to update curAlphaColumn
        //          after the value of curX.
        // finalX -- the minimum of all the nextX values.
        //
        // curX advances to finalX outputting any blocks that it passes along
        // the way. Since finalX will not change when we reach the end of a
        // run, the termination criteria will be whenever curX == finalX at the
        // end of a loop.

        // Setup:
        Block block;
        sk_bzero(block, sizeof(block));

        Column curAlphaColumn;
        sk_bzero(curAlphaColumn, sizeof(curAlphaColumn));

        SkAlpha *curAlpha = reinterpret_cast<SkAlpha*>(&curAlphaColumn);

        int nextX[BlockDim];
        for (int i = 0; i < BlockDim; ++i) {
            nextX[i] = 0x7FFFFF;
        }

        uint8_t* outPtr = this->getBlock(fBufferedRuns[0].fX, fBufferedRuns[0].fY);

        // Populate the first set of runs and figure out how far we need to
        // advance on the first step
        int curX = 0;
        int finalX = 0xFFFFF;
        for (int i = 0; i < BlockDim; ++i) {
            nextX[i] = *(fBufferedRuns[i].fRuns);
            curAlpha[i] = *(fBufferedRuns[i].fAlphas);

            finalX = SkMin32(nextX[i], finalX);
        }

        // Make sure that we have a valid right-bound X value
        SkASSERT(finalX < 0xFFFFF);

        // If the finalX is the longest run, then just blit until we have
        // width...
        if (kLongestRun == finalX) {
            finalX = fWidth;
        }

        // Run the blitter...
        while (curX != finalX) {
            SkASSERT(finalX >= curX);

            // Do we need to populate the rest of the block?
            if ((finalX - (BlockDim*(curX / BlockDim))) >= BlockDim) {
                const int col = curX % BlockDim;
                const int colsLeft = BlockDim - col;
                SkASSERT(curX + colsLeft <= finalX);

                this->updateBlockColumns(block, col, colsLeft, curAlphaColumn);

                // Write this block
                CompressorType::CompressA8Vertical(outPtr, reinterpret_cast<uint8_t*>(block));
                outPtr += EncodedBlockSize;
                curX += colsLeft;
            }

            // If we can advance even further, then just keep memsetting the block
            if ((finalX - curX) >= BlockDim) {
                SkASSERT((curX % BlockDim) == 0);

                const int col = 0;
                const int colsLeft = BlockDim;

                this->updateBlockColumns(block, col, colsLeft, curAlphaColumn);

                // While we can keep advancing, just keep writing the block.
                uint8_t lastBlock[EncodedBlockSize];
                CompressorType::CompressA8Vertical(lastBlock, reinterpret_cast<uint8_t*>(block));
                while((finalX - curX) >= BlockDim) {
                    memcpy(outPtr, lastBlock, EncodedBlockSize);
                    outPtr += EncodedBlockSize;
                    curX += BlockDim;
                }
            }

            // If we haven't advanced within the block then do so.
            if (curX < finalX) {
                const int col = curX % BlockDim;
                const int colsLeft = finalX - curX;

                this->updateBlockColumns(block, col, colsLeft, curAlphaColumn);
                curX += colsLeft;
            }

            SkASSERT(curX == finalX);

            // Figure out what the next advancement is...
            if (finalX < fWidth) {
                for (int i = 0; i < BlockDim; ++i) {
                    if (nextX[i] == finalX) {
                        const int16_t run = *(fBufferedRuns[i].fRuns);
                        fBufferedRuns[i].fRuns += run;
                        fBufferedRuns[i].fAlphas += run;
                        curAlpha[i] = *(fBufferedRuns[i].fAlphas);
                        nextX[i] += *(fBufferedRuns[i].fRuns);
                    }
                }

                finalX = 0xFFFFF;
                for (int i = 0; i < BlockDim; ++i) {
                    finalX = SkMin32(nextX[i], finalX);
                }
            } else {
                curX = finalX;
            }
        }

        // If we didn't land on a block boundary, output the block...
        if ((curX % BlockDim) > 0) {
#ifdef SK_DEBUG
            for (int i = 0; i < BlockDim; ++i) {
                SkASSERT(nextX[i] == kLongestRun || nextX[i] == curX);
            }
#endif
            const int col = curX % BlockDim;
            const int colsLeft = BlockDim - col;

            memset(curAlphaColumn, 0, sizeof(curAlphaColumn));
            this->updateBlockColumns(block, col, colsLeft, curAlphaColumn);

            CompressorType::CompressA8Vertical(outPtr, reinterpret_cast<uint8_t*>(block));
        }

        fNextRun = 0;
    }

#if PEDANTIC_BLIT_RECT
    void updateBlockRow(int x, int y, int width, int height,
                        int blockRow, int startBlockX, int endBlockX) {
        if (0 == width || 0 == height || startBlockX == endBlockX) {
            return;
        }

        uint8_t* dst = this->getBlock(startBlockX, BlockDim * (y / BlockDim));

        // One horizontal strip to update
        uint8_t mask[BlockDim*BlockDim];
        memset(mask, 0, sizeof(mask));

        // Update the left cap
        int blockX = startBlockX;
        const int yoff = y - blockRow;
        for (int j = 0; j < height; ++j) {
            const int xoff = x - blockX;
            memset(mask + (j + yoff)*BlockDim + xoff, 0xFF, BlockDim - xoff);
        }
        CompressorType::UpdateBlock(dst, mask, BlockDim, mask);
        dst += EncodedBlockSize;
        blockX += BlockDim;

        // Update the middle
        if (blockX < endBlockX) {
            for (int j = 0; j < height; ++j) {
                memset(mask + (j + yoff)*BlockDim, 0xFF, BlockDim);
            }
            while (blockX < endBlockX) {
                CompressorType::UpdateBlock(dst, mask, BlockDim, mask);
                dst += EncodedBlockSize;
                blockX += BlockDim;
            }
        }

        SkASSERT(endBlockX == blockX);

        // Update the right cap (if we need to)
        if (x + width > endBlockX) {
            memset(mask, 0, sizeof(mask));
            for (int j = 0; j < height; ++j) {
                const int xoff = (x+width-blockX);
                memset(mask + (j+yoff)*BlockDim, 0xFF, xoff);
            }
            CompressorType::UpdateBlock(dst, mask, BlockDim, mask);
        }
    }

    void updateBlockCol(int x, int y, int width, int height,
                        int blockCol, int startBlockY, int endBlockY) {
        if (0 == width || 0 == height || startBlockY == endBlockY) {
            return;
        }

        // One vertical strip to update
        uint8_t mask[BlockDim*BlockDim];
        memset(mask, 0, sizeof(mask));
        const int maskX0 = x - blockCol;
        const int maskWidth = maskX0 + width;
        SkASSERT(maskWidth <= BlockDim);

        // Update the top cap
        int blockY = startBlockY;
        for (int j = (y - blockY); j < BlockDim; ++j) {
            memset(mask + maskX0 + j*BlockDim, 0xFF, maskWidth);
        }
        CompressorType::UpdateBlock(this->getBlock(blockCol, blockY), mask, BlockDim, mask);
        blockY += BlockDim;

        // Update middle
        if (blockY < endBlockY) {
            for (int j = 0; j < BlockDim; ++j) {
                memset(mask + maskX0 + j*BlockDim, 0xFF, maskWidth);
            }
            while (blockY < endBlockY) {
                CompressorType::UpdateBlock(this->getBlock(blockCol, blockY),
                                            mask, BlockDim, mask);
                blockY += BlockDim;
            }
        }

        SkASSERT(endBlockY == blockY);

        // Update bottom
        if (y + height > endBlockY) {
            for (int j = y+height; j < endBlockY + BlockDim; ++j) {
                memset(mask + (j-endBlockY)*BlockDim, 0, BlockDim);
            }
            CompressorType::UpdateBlock(this->getBlock(blockCol, blockY),
                                        mask, BlockDim, mask);
        }
    }
#endif  // PEDANTIC_BLIT_RECT

};

}  // namespace SkTextureCompressor

#endif  // SkTextureCompressor_Blitter_DEFINED
