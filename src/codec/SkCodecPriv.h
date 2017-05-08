/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecPriv_DEFINED
#define SkCodecPriv_DEFINED

#include "SkColorPriv.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorTable.h"
#include "SkEncodedInfo.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

#ifdef SK_PRINT_CODEC_MESSAGES
    #define SkCodecPrintf SkDebugf
#else
    #define SkCodecPrintf(...)
#endif

// FIXME: Consider sharing with dm, nanbench, and tools.
static inline float get_scale_from_sample_size(int sampleSize) {
    return 1.0f / ((float) sampleSize);
}

static inline bool is_valid_subset(const SkIRect& subset, const SkISize& imageDims) {
    return SkIRect::MakeSize(imageDims).contains(subset);
}

/*
 * returns a scaled dimension based on the original dimension and the sampleSize
 * NOTE: we round down here for scaled dimension to match the behavior of SkImageDecoder
 * FIXME: I think we should call this get_sampled_dimension().
 */
static inline int get_scaled_dimension(int srcDimension, int sampleSize) {
    if (sampleSize > srcDimension) {
        return 1;
    }
    return srcDimension / sampleSize;
}

/*
 * Returns the first coordinate that we will keep during a scaled decode.
 * The output can be interpreted as an x-coordinate or a y-coordinate.
 *
 * This does not need to be called and is not called when sampleFactor == 1.
 */
static inline int get_start_coord(int sampleFactor) { return sampleFactor / 2; };

/*
 * Given a coordinate in the original image, this returns the corresponding
 * coordinate in the scaled image.  This function is meaningless if
 * IsCoordNecessary returns false.
 * The output can be interpreted as an x-coordinate or a y-coordinate.
 *
 * This does not need to be called and is not called when sampleFactor == 1.
 */
static inline int get_dst_coord(int srcCoord, int sampleFactor) { return srcCoord / sampleFactor; };

/*
 * When scaling, we will discard certain y-coordinates (rows) and
 * x-coordinates (columns).  This function returns true if we should keep the
 * coordinate and false otherwise.
 * The inputs may be x-coordinates or y-coordinates.
 *
 * This does not need to be called and is not called when sampleFactor == 1.
 */
static inline bool is_coord_necessary(int srcCoord, int sampleFactor, int scaledDim) {
    // Get the first coordinate that we want to keep
    int startCoord = get_start_coord(sampleFactor);

    // Return false on edge cases
    if (srcCoord < startCoord || get_dst_coord(srcCoord, sampleFactor) >= scaledDim) {
        return false;
    }

    // Every sampleFactor rows are necessary
    return ((srcCoord - startCoord) % sampleFactor) == 0;
}

static inline bool valid_alpha(SkAlphaType dstAlpha, SkAlphaType srcAlpha) {
    if (kUnknown_SkAlphaType == dstAlpha) {
        return false;
    }

    if (srcAlpha != dstAlpha) {
        if (kOpaque_SkAlphaType == srcAlpha) {
            // If the source is opaque, we can support any.
            SkCodecPrintf("Warning: an opaque image should be decoded as opaque "
                          "- it is being decoded as non-opaque, which will draw slower\n");
            return true;
        }

        // The source is not opaque
        switch (dstAlpha) {
            case kPremul_SkAlphaType:
            case kUnpremul_SkAlphaType:
                // The source is not opaque, so either of these is okay
                break;
            default:
                // We cannot decode a non-opaque image to opaque (or unknown)
                return false;
        }
    }
    return true;
}

/*
 * If there is a color table, get a pointer to the colors, otherwise return nullptr
 */
static inline const SkPMColor* get_color_ptr(SkColorTable* colorTable) {
     return nullptr != colorTable ? colorTable->readColors() : nullptr;
}

/*
 * Given that the encoded image uses a color table, return the fill value
 */
static inline uint64_t get_color_table_fill_value(SkColorType dstColorType, SkAlphaType alphaType,
        const SkPMColor* colorPtr, uint8_t fillIndex, SkColorSpaceXform* colorXform, bool isRGBA) {
    SkASSERT(nullptr != colorPtr);
    switch (dstColorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return colorPtr[fillIndex];
        case kRGB_565_SkColorType:
            return SkPixel32ToPixel16(colorPtr[fillIndex]);
        case kIndex_8_SkColorType:
            return fillIndex;
        case kRGBA_F16_SkColorType: {
            SkASSERT(colorXform);
            uint64_t dstColor;
            uint32_t srcColor = colorPtr[fillIndex];
            SkColorSpaceXform::ColorFormat srcFormat =
                    isRGBA ? SkColorSpaceXform::kRGBA_8888_ColorFormat
                           : SkColorSpaceXform::kBGRA_8888_ColorFormat;
            SkAssertResult(colorXform->apply(select_xform_format(dstColorType), &dstColor,
                                             srcFormat, &srcColor, 1, alphaType));
            return dstColor;
        }
        default:
            SkASSERT(false);
            return 0;
    }
}

/*
 *
 * Copy the codec color table back to the client when kIndex8 color type is requested
 */
static inline void copy_color_table(const SkImageInfo& dstInfo, SkColorTable* colorTable,
        SkPMColor* inputColorPtr, int* inputColorCount) {
    if (kIndex_8_SkColorType == dstInfo.colorType()) {
        SkASSERT(nullptr != inputColorPtr);
        SkASSERT(nullptr != inputColorCount);
        SkASSERT(nullptr != colorTable);
        memcpy(inputColorPtr, colorTable->readColors(), *inputColorCount * sizeof(SkPMColor));
    }
}

/*
 * Compute row bytes for an image using pixels per byte
 */
static inline size_t compute_row_bytes_ppb(int width, uint32_t pixelsPerByte) {
    return (width + pixelsPerByte - 1) / pixelsPerByte;
}

/*
 * Compute row bytes for an image using bytes per pixel
 */
static inline size_t compute_row_bytes_bpp(int width, uint32_t bytesPerPixel) {
    return width * bytesPerPixel;
}

/*
 * Compute row bytes for an image
 */
static inline size_t compute_row_bytes(int width, uint32_t bitsPerPixel) {
    if (bitsPerPixel < 16) {
        SkASSERT(0 == 8 % bitsPerPixel);
        const uint32_t pixelsPerByte = 8 / bitsPerPixel;
        return compute_row_bytes_ppb(width, pixelsPerByte);
    } else {
        SkASSERT(0 == bitsPerPixel % 8);
        const uint32_t bytesPerPixel = bitsPerPixel / 8;
        return compute_row_bytes_bpp(width, bytesPerPixel);
    }
}

/*
 * Get a byte from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint8_t get_byte(uint8_t* buffer, uint32_t i) {
    return buffer[i];
}

/*
 * Get a short from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint16_t get_short(uint8_t* buffer, uint32_t i) {
    uint16_t result;
    memcpy(&result, &(buffer[i]), 2);
#ifdef SK_CPU_BENDIAN
    return SkEndianSwap16(result);
#else
    return result;
#endif
}

/*
 * Get an int from a buffer
 * This method is unsafe, the caller is responsible for performing a check
 */
static inline uint32_t get_int(uint8_t* buffer, uint32_t i) {
    uint32_t result;
    memcpy(&result, &(buffer[i]), 4);
#ifdef SK_CPU_BENDIAN
    return SkEndianSwap32(result);
#else
    return result;
#endif
}

/*
 * @param data           Buffer to read bytes from
 * @param isLittleEndian Output parameter
 *                       Indicates if the data is little endian
 *                       Is unaffected on false returns
 */
static inline bool is_valid_endian_marker(const uint8_t* data, bool* isLittleEndian) {
    // II indicates Intel (little endian) and MM indicates motorola (big endian).
    if (('I' != data[0] || 'I' != data[1]) && ('M' != data[0] || 'M' != data[1])) {
        return false;
    }

    *isLittleEndian = ('I' == data[0]);
    return true;
}

static inline uint16_t get_endian_short(const uint8_t* data, bool littleEndian) {
    if (littleEndian) {
        return (data[1] << 8) | (data[0]);
    }

    return (data[0] << 8) | (data[1]);
}

static inline SkPMColor premultiply_argb_as_rgba(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }

    return SkPackARGB_as_RGBA(a, r, g, b);
}

static inline SkPMColor premultiply_argb_as_bgra(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }

    return SkPackARGB_as_BGRA(a, r, g, b);
}

static inline bool is_rgba(SkColorType colorType) {
#ifdef SK_PMCOLOR_IS_RGBA
    return (kBGRA_8888_SkColorType != colorType);
#else
    return (kRGBA_8888_SkColorType == colorType);
#endif
}

// Method for coverting to a 32 bit pixel.
typedef uint32_t (*PackColorProc)(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

static inline PackColorProc choose_pack_color_proc(bool isPremul, SkColorType colorType) {
    bool isRGBA = is_rgba(colorType);
    if (isPremul) {
        if (isRGBA) {
            return &premultiply_argb_as_rgba;
        } else {
            return &premultiply_argb_as_bgra;
        }
    } else {
        if (isRGBA) {
            return &SkPackARGB_as_RGBA;
        } else {
            return &SkPackARGB_as_BGRA;
        }
    }
}

static inline bool needs_premul(const SkImageInfo& dstInfo, const SkEncodedInfo& encodedInfo) {
    return kPremul_SkAlphaType == dstInfo.alphaType() &&
           SkEncodedInfo::kUnpremul_Alpha == encodedInfo.alpha();
}

static inline bool needs_color_xform(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo,
                                     bool needsColorCorrectPremul) {
    // We never perform a color xform in legacy mode.
    if (!dstInfo.colorSpace()) {
        return false;
    }

    // F16 is by definition a linear space, so we always must perform a color xform.
    bool isF16 = kRGBA_F16_SkColorType == dstInfo.colorType();

    // Need a color xform when dst space does not match the src.
    bool srcDstNotEqual = !SkColorSpace::Equals(srcInfo.colorSpace(), dstInfo.colorSpace());

    return needsColorCorrectPremul || isF16 || srcDstNotEqual;
}

static inline SkAlphaType select_xform_alpha(SkAlphaType dstAlphaType, SkAlphaType srcAlphaType) {
    return (kOpaque_SkAlphaType == srcAlphaType) ? kOpaque_SkAlphaType : dstAlphaType;
}

static inline bool apply_xform_on_decode(SkColorType dstColorType, SkEncodedInfo::Color srcColor) {
    // We will apply the color xform when reading the color table unless F16 is requested.
    return SkEncodedInfo::kPalette_Color != srcColor || kRGBA_F16_SkColorType == dstColorType;
}

/*
 * Alpha Type Conversions
 * - kOpaque to kOpaque, kUnpremul, kPremul is valid
 * - kUnpremul to kUnpremul, kPremul is valid
 *
 * Color Type Conversions
 * - Always support kRGBA_8888, kBGRA_8888
 * - Support kRGBA_F16 when there is a linear dst color space
 * - Support kIndex8 if it matches the src
 * - Support k565 if kOpaque and color correction is not required
 * - Support k565 if it matches the src, kOpaque, and color correction is not required
 */
static inline bool conversion_possible(const SkImageInfo& dst, const SkImageInfo& src) {
    // Ensure the alpha type is valid.
    if (!valid_alpha(dst.alphaType(), src.alphaType())) {
        return false;
    }

    // Check for supported color types.
    switch (dst.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        case kRGBA_F16_SkColorType:
            return dst.colorSpace() && dst.colorSpace()->gammaIsLinear();
        case kIndex_8_SkColorType:
            return kIndex_8_SkColorType == src.colorType();
        case kRGB_565_SkColorType:
            return kOpaque_SkAlphaType == src.alphaType();
        case kGray_8_SkColorType:
            return kGray_8_SkColorType == src.colorType() &&
                   kOpaque_SkAlphaType == src.alphaType() && !needs_color_xform(dst, src, false);
        default:
            return false;
    }
}

static inline SkColorSpaceXform::ColorFormat select_xform_format_ct(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
        case kBGRA_8888_SkColorType:
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
        case kRGB_565_SkColorType:
        case kIndex_8_SkColorType:
#ifdef SK_PMCOLOR_IS_RGBA
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
#else
            return SkColorSpaceXform::kBGRA_8888_ColorFormat;
#endif
        default:
            SkASSERT(false);
            return SkColorSpaceXform::kRGBA_8888_ColorFormat;
    }
}

#endif // SkCodecPriv_DEFINED
