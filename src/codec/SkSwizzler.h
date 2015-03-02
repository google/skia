/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSwizzler_DEFINED
#define SkSwizzler_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "SkImageInfo.h"

class SkSwizzler : public SkNoncopyable {
public:
    /**
     *  Enum describing the config of the source data.
     */
    enum SrcConfig {
        kGray,  // 1 byte per pixel
        kIndex, // 1 byte per pixel
        kRGB,   // 3 bytes per pixel
        kRGBX,  // 4 byes per pixel (ignore 4th)
        kRGBA,  // 4 bytes per pixel
        kRGB_565 // 2 bytes per pixel
    };

    static int BytesPerPixel(SrcConfig sc) {
        switch (sc) {
            case kGray:
            case kIndex:
                return 1;
            case kRGB:
                return 3;
            case kRGBX:
            case kRGBA:
                return 4;
            case kRGB_565:
                return 2;
            default:
                SkDebugf("invalid source config passed to BytesPerPixel\n");
                return -1;
        }
    }

    /**
     *  Create a new SkSwizzler.
     *  @param sc SrcConfig
     *  @param info dimensions() describe both the src and the dst.
     *              Other fields describe the dst.
     *  @param dst Destination to write pixels. Must match info and dstRowBytes
     *  @param dstRowBytes rowBytes for dst.
     *  @param skipZeroes Whether to skip writing zeroes. Useful if dst is
     *              zero-initialized. The implementation may or may not respect this.
     *  @return A new SkSwizzler or NULL on failure.
     */
    static SkSwizzler* CreateSwizzler(SrcConfig sc, const SkPMColor* ctable,
                                      const SkImageInfo& info, void* dst,
                                      size_t dstRowBytes, bool skipZeroes);
    /**
     *  Swizzle the next line. Call height times, once for each row of source.
     *  @param src The next row of the source data.
     *  @return Whether the row had non-opaque alpha.
     */
    bool next(const uint8_t* SK_RESTRICT src);
private:
    /**
     *  Method for converting raw data to Skia pixels.
     *  @param dstRow Row in which to write the resulting pixels.
     *  @param src Row of src data, in format specified by SrcConfig
     *  @param width Width in pixels
     *  @param bpp bytes per pixel of the source.
     *  @param y Line of source.
     *  @param ctable Colors (used for kIndex source).
     */
    typedef bool (*RowProc)(void* SK_RESTRICT dstRow,
                            const uint8_t* SK_RESTRICT src,
                            int width, int bpp, int y,
                            const SkPMColor ctable[]);

    const RowProc       fRowProc;
    const SkPMColor*    fColorTable;    // Unowned pointer
    const int           fSrcPixelSize;
    const SkImageInfo   fDstInfo;
    void*               fDstRow;
    const size_t        fDstRowBytes;
    int                 fCurrY;

    SkSwizzler(RowProc proc, const SkPMColor* ctable, int srcBpp,
               const SkImageInfo& info, void* dst, size_t rowBytes);

};
#endif // SkSwizzler_DEFINED
