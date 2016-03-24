
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkScaledBitmapSampler_DEFINED
#define SkScaledBitmapSampler_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "SkImageDecoder.h"

class SkBitmap;

class SkScaledBitmapSampler {
public:
    SkScaledBitmapSampler(int origWidth, int origHeight, int cellSize);

    int scaledWidth() const { return fScaledWidth; }
    int scaledHeight() const { return fScaledHeight; }

    int srcY0() const { return fY0; }
    int srcDX() const { return fDX; }
    int srcDY() const { return fDY; }

    enum SrcConfig {
        kGray,  // 1 byte per pixel
        kIndex, // 1 byte per pixel
        kRGB,   // 3 bytes per pixel
        kRGBX,  // 4 byes per pixel (ignore 4th)
        kRGBA,  // 4 bytes per pixel
        kRGB_565 // 2 bytes per pixel
    };

    struct Options {
        bool fDither;
        bool fPremultiplyAlpha;
        bool fSkipZeros;
        explicit Options(const SkImageDecoder &dec)
            : fDither(dec.getDitherImage())
            , fPremultiplyAlpha(!dec.getRequireUnpremultipliedColors())
            , fSkipZeros(dec.getSkipWritingZeroes())
            { }
    };

    // Given a dst bitmap (with pixels already allocated) and a src-config,
    // prepares iterator to process the src colors and write them into dst.
    // Returns false if the request cannot be fulfulled.
    bool begin(SkBitmap* dst, SrcConfig sc, const SkImageDecoder& decoder,
               const SkPMColor* = nullptr);
    bool begin(SkBitmap* dst, SrcConfig sc, const Options& opts,
               const SkPMColor* = nullptr);
    // call with row of src pixels, for y = 0...scaledHeight-1.
    // returns true if the row had non-opaque alpha in it
    bool next(const uint8_t* SK_RESTRICT src);

    // Like next(), but specifies the y value of the source row, so the
    // rows can come in any order. If the row is not part of the output
    // sample, it will be skipped. Only sampleInterlaced OR next should
    // be called for one SkScaledBitmapSampler.
    bool sampleInterlaced(const uint8_t* SK_RESTRICT src, int srcY);

    typedef bool (*RowProc)(void* SK_RESTRICT dstRow,
                            const uint8_t* SK_RESTRICT src,
                            int width, int deltaSrc, int y,
                            const SkPMColor[]);

private:
    int fScaledWidth;
    int fScaledHeight;

    int fX0;    // first X coord to sample
    int fY0;    // first Y coord (scanline) to sample
    int fDX;    // step between X samples
    int fDY;    // step between Y samples

#ifdef SK_DEBUG
    // Keep track of whether the caller is using next or sampleInterlaced.
    // Only one can be used per sampler.
    enum SampleMode {
        kUninitialized_SampleMode,
        kConsecutive_SampleMode,
        kInterlaced_SampleMode,
    };

    SampleMode fSampleMode;
#endif

    // setup state
    char*   fDstRow; // points into bitmap's pixels
    size_t  fDstRowBytes;
    int     fCurrY; // used for dithering
    int     fSrcPixelSize;  // 1, 3, 4
    RowProc fRowProc;

    // optional reference to the src colors if the src is a palette model
    const SkPMColor* fCTable;

#ifdef SK_DEBUG
    // Helper class allowing a test to have access to fRowProc.
    friend class RowProcTester;
#endif
};

#endif
