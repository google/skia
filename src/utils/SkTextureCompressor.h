/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextureCompressor_DEFINED
#define SkTextureCompressor_DEFINED

#include "SkImageInfo.h"
#include "SkBlitter.h"

class SkBitmap;
class SkData;

namespace SkTextureCompressor {
    // Various texture compression formats that we support.
    enum Format {
        // Alpha only formats.
        kLATC_Format,
        kR11_EAC_Format,

        kLast_Format = kR11_EAC_Format
    };
    static const int kFormatCnt = kLast_Format + 1;

    // Returns an SkData holding a blob of compressed data that corresponds
    // to the bitmap. If the bitmap colorType cannot be compressed using the 
    // associated format, then we return NULL. The caller is responsible for
    // calling unref() on the returned data.
    SkData* CompressBitmapToFormat(const SkBitmap& bitmap, Format format);

    // Compresses the given src data into dst. The src data is assumed to be
    // large enough to hold width*height pixels. The dst data is expected to
    // be large enough to hold the compressed data according to the format.
    bool CompressBufferToFormat(uint8_t* dst, const uint8_t* src, SkColorType srcColorType,
                                int width, int height, int rowBytes, Format format,
                                bool opt = true /* Use optimization if available */);

    // This typedef defines what the nominal aspects of a compression function
    // are. The typedef is not meant to be used by clients of the API, but rather
    // allows SIMD optimized compression functions to be implemented.
    typedef bool (*CompressionProc)(uint8_t* dst, const uint8_t* src,
                                    int width, int height, int rowBytes);

    // This class implements a blitter that blits directly into a buffer that will
    // be used as an R11 EAC compressed texture. We compute this buffer by
    // buffering four scan lines and then outputting them all at once. This blitter
    // is only expected to be used with alpha masks, i.e. kAlpha8_SkColorType.
    class R11_EACBlitter : public SkBlitter {
    public:
        R11_EACBlitter(int width, int height, void *compressedBuffer);
        virtual ~R11_EACBlitter() { this->flushRuns(); }

        // Blit a horizontal run of one or more pixels.
        virtual void blitH(int x, int y, int width) SK_OVERRIDE {
            // This function is intended to be called from any standard RGB
            // buffer, so we should never encounter it. However, if some code
            // path does end up here, then this needs to be investigated.
            SkFAIL("Not implemented!");
        }

        /// Blit a horizontal run of antialiased pixels; runs[] is a *sparse*
        /// zero-terminated run-length encoding of spans of constant alpha values.
        virtual void blitAntiH(int x, int y,
                               const SkAlpha antialias[],
                               const int16_t runs[]) SK_OVERRIDE;

        // Blit a vertical run of pixels with a constant alpha value.
        virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE {
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

        // Blit a solid rectangle one or more pixels wide.
        virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE {
            // Analogous to blitRow, this function is intended for RGB targets
            // and should never be called by this blitter. Any calls to this function
            // are probably a bug and should be investigated.
            SkFAIL("Not implemented!");
        }

        // Blit a rectangle with one alpha-blended column on the left,
        // width (zero or more) opaque pixels, and one alpha-blended column
        // on the right. The result will always be at least two pixels wide.
        virtual void blitAntiRect(int x, int y, int width, int height,
                                  SkAlpha leftAlpha, SkAlpha rightAlpha) SK_OVERRIDE {
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

        // Blit a pattern of pixels defined by a rectangle-clipped mask;
        // typically used for text.
        virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE {
            // This function is currently not implemented. It is not explicitly
            // required by the contract, but if at some time a code path runs into
            // this function (which is entirely possible), it needs to be implemented.
            //
            // TODO (krajcevski):
            // This function will be most easily implemented in the same way as
            // blitAntiRect above.
            SkFAIL("Not implemented!");
        }

        // If the blitter just sets a single value for each pixel, return the
        // bitmap it draws into, and assign value. If not, return NULL and ignore
        // the value parameter.
        virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE {
            return NULL;
        }

        /**
         * Compressed texture blitters only really work correctly if they get
         * four blocks at a time. That being said, this blitter tries it's best
         * to preserve semantics if blitAntiH doesn't get called in too many
         * weird ways...
         */
        virtual int requestRowsPreserved() const { return kR11_EACBlockSz; }

    protected:
        virtual void onNotifyFinished() { this->flushRuns(); }

    private:
        static const int kR11_EACBlockSz = 4;
        static const int kPixelsPerBlock = kR11_EACBlockSz * kR11_EACBlockSz;

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
        } fBufferedRuns[kR11_EACBlockSz];

        // The next row (0-3) that we need to blit. This value should never exceed
        // the number of rows that we have (kR11_EACBlockSz)
        int fNextRun;

        // The width and height of the image that we're blitting
        const int fWidth;
        const int fHeight;

        // The R11 EAC buffer that we're blitting into. It is assumed that the buffer
        // is large enough to store a compressed image of size fWidth*fHeight.
        uint64_t* const fBuffer;

        // Various utility functions
        int blocksWide() const { return fWidth / kR11_EACBlockSz; }
        int blocksTall() const { return fHeight / kR11_EACBlockSz; }
        int totalBlocks() const { return (fWidth * fHeight) / kPixelsPerBlock; }

        // Returns the block index for the block containing pixel (x, y). Block
        // indices start at zero and proceed in raster order.
        int getBlockOffset(int x, int y) const {
            SkASSERT(x < fWidth);
            SkASSERT(y < fHeight);
            const int blockCol = x / kR11_EACBlockSz;
            const int blockRow = y / kR11_EACBlockSz;
            return blockRow * this->blocksWide() + blockCol;
        }

        // Returns a pointer to the block containing pixel (x, y)
        uint64_t *getBlock(int x, int y) const {
            return fBuffer + this->getBlockOffset(x, y);
        }

        // The following function writes the buffered runs to compressed blocks.
        // If fNextRun < 4, then we fill the runs that we haven't buffered with
        // the constant zero buffer.
        void flushRuns();
    };
}

#endif
