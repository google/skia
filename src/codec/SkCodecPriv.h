/*
 * Copyright 2015 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecPriv_DEFINED
#define SkCodecPriv_DEFINED

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkColorPalette.h"
#include "src/core/SkColorData.h"

#include <string_view>

#ifdef SK_PRINT_CODEC_MESSAGES
    #define SkCodecPrintf SkDebugf
#else
    #define SkCodecPrintf(...)
#endif

namespace SkCodecs {

bool HasDecoder(std::string_view id);

// All color profile information from an SkCodec.
// TODO(https://issues.skia.org/issues/464217864): This should include skhdr::Metadata.
class ColorProfile {
public:
    // Create a ColorProfile from an SkData for an ICC profile.
    // Returns nullptr if the data fails to parse. This will use
    // either MakeICCProfileWithSkCMS or MakeICCProfileWithRust
    // depending on the build configuration.
    static std::unique_ptr<ColorProfile> MakeICCProfile(sk_sp<const SkData>);

    // Implementation of MakeICCProfile that uses skcms to parse.
    // This should only be called by tests.
    static std::unique_ptr<ColorProfile> MakeICCProfileWithSkCMS(sk_sp<const SkData>);

    // Create a ColorProfile from an SkColorSpace. Returns nullptr if the color space is
    // nullptr.
    static std::unique_ptr<ColorProfile> Make(sk_sp<SkColorSpace>);

    // Create a ColorProfile from the specified matrix and transfer curves. This will always
    // succeed.
    static std::unique_ptr<ColorProfile> Make(const skcms_TransferFunction& trfn,
                                              const skcms_Matrix3x3& toXYZD50);

    // Create a ColorProfile with the specified CICP values. This will always succeed, even if
    // the values are unrecognized.
    static std::unique_ptr<ColorProfile> MakeCICP(uint8_t color_primaries,
                                                  uint8_t transfer_characteristics,
                                                  uint8_t matrix_coefficients,
                                                  uint8_t full_range_flag);

    // Create a copy of this.
    std::unique_ptr<ColorProfile> clone() const;

    // Return the color data space for the profile.
    enum class DataSpace {
        kRGB,   // skcms_Signature_RGB
        kCMYK,  // skcms_Signature_CMYK
        kGray,  // skcms_Signature_Gray
        kOther, // all other values
    };
    DataSpace dataSpace() const;

    // Return the color space that this profile represents exactly. Return nullptr if this profile
    // cannot be represented as an SkColorSpace.
    sk_sp<SkColorSpace> getExactColorSpace() const;

    // Return the color space that Android uses for this profile (see implementation for details).
    // Will not return nullptr.
    sk_sp<SkColorSpace> getAndroidOutputColorSpace() const;

    // TODO(https://issues.skia.org/issues/464217864): Remove direct access to the
    // skcms_ICCProfile and change data() to be a serialize() function.
    const skcms_ICCProfile* profile() const { return &fProfile; }
    sk_sp<const SkData> data() const { return fData; }

private:
    ColorProfile(const skcms_ICCProfile&, sk_sp<const SkData> = nullptr);

#if defined(SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST)
    friend std::unique_ptr<ColorProfile> MakeICCProfileWithRust(sk_sp<const SkData>);
#endif

    skcms_ICCProfile     fProfile;
    sk_sp<const SkData>  fData;
};

}  // namespace SkCodecs

class SkCodecPriv final {
public:
    static const SkEncodedInfo& GetEncodedInfo(const SkCodec* codec) {
        SkASSERT(codec);
        return codec->getEncodedInfo();
    }

    static bool SelectXformFormat(SkColorType colorType,
                                  bool forColorTable,
                                  skcms_PixelFormat* outFormat);

    // FIXME: Consider sharing with dm, nanbench, and tools.
    static float GetScaleFromSampleSize(int sampleSize) { return 1.0f / ((float)sampleSize); }

    static bool IsValidSubset(const SkIRect& subset, const SkISize& imageDims) {
        return SkIRect::MakeSize(imageDims).contains(subset);
    }

    /*
     * returns a scaled dimension based on the original dimension and the sampleSize
     * NOTE: we round down here for scaled dimension to match the behavior of SkImageDecoder
     */
    static int GetSampledDimension(int srcDimension, int sampleSize) {
        if (sampleSize > srcDimension) {
            return 1;
        }
        if (sampleSize == 0) {
            return 0;
        }
        return srcDimension / sampleSize;
    }

    /*
     * Returns the first coordinate that we will keep during a scaled decode.
     * The output can be interpreted as an x-coordinate or a y-coordinate.
     *
     * This does not need to be called and is not called when sampleFactor == 1.
     */
    static int GetStartCoord(int sampleFactor) { return sampleFactor / 2; }

    /*
     * Given a coordinate in the original image, this returns the corresponding
     * coordinate in the scaled image.  This function is meaningless if
     * IsCoordNecessary returns false.
     * The output can be interpreted as an x-coordinate or a y-coordinate.
     *
     * This does not need to be called and is not called when sampleFactor == 1.
     */
    static int GetDstCoord(int srcCoord, int sampleFactor) { return srcCoord / sampleFactor; }

    /*
     * When scaling, we will discard certain y-coordinates (rows) and
     * x-coordinates (columns).  This function returns true if we should keep the
     * coordinate and false otherwise.
     * The inputs may be x-coordinates or y-coordinates.
     *
     * This does not need to be called and is not called when sampleFactor == 1.
     */
    static bool IsCoordNecessary(int srcCoord, int sampleFactor, int scaledDim) {
        // Get the first coordinate that we want to keep
        int startCoord = GetStartCoord(sampleFactor);

        // Return false on edge cases
        if (srcCoord < startCoord || GetDstCoord(srcCoord, sampleFactor) >= scaledDim) {
            return false;
        }

        // Every sampleFactor rows are necessary
        return ((srcCoord - startCoord) % sampleFactor) == 0;
    }

    static bool ValidAlpha(SkAlphaType dstAlpha, bool srcIsOpaque) {
        if (kUnknown_SkAlphaType == dstAlpha) {
            return false;
        }

        if (srcIsOpaque) {
            if (kOpaque_SkAlphaType != dstAlpha) {
                SkCodecPrintf(
                        "Warning: an opaque image should be decoded as opaque "
                        "- it is being decoded as non-opaque, which will draw slower\n");
            }
            return true;
        }

        return dstAlpha != kOpaque_SkAlphaType;
    }

    /*
     * If there is a color table, get a pointer to the colors, otherwise return nullptr
     */
    static const SkPMColor* GetColorPtr(SkColorPalette* colorTable) {
        return nullptr != colorTable ? colorTable->readColors() : nullptr;
    }

    /*
     * Compute row bytes for an image using pixels per byte
     */
    static size_t ComputeRowBytesPixelsPerByte(int width, uint32_t pixelsPerByte) {
        return (width + pixelsPerByte - 1) / pixelsPerByte;
    }

    /*
     * Compute row bytes for an image using bytes per pixel
     */
    static size_t ComputeRowBytesBytesPerPixel(int width, uint32_t bytesPerPixel) {
        return width * bytesPerPixel;
    }

    /*
     * Compute row bytes for an image
     */
    static size_t ComputeRowBytes(int width, uint32_t bitsPerPixel) {
        if (bitsPerPixel < 16) {
            SkASSERT(0 == 8 % bitsPerPixel);
            const uint32_t pixelsPerByte = 8 / bitsPerPixel;
            return ComputeRowBytesPixelsPerByte(width, pixelsPerByte);
        } else {
            SkASSERT(0 == bitsPerPixel % 8);
            const uint32_t bytesPerPixel = bitsPerPixel / 8;
            return ComputeRowBytesBytesPerPixel(width, bytesPerPixel);
        }
    }

    /*
     * Get a byte from a buffer
     * This method is unsafe, the caller is responsible for performing a check
     */
    static uint8_t UnsafeGetByte(const uint8_t* buffer, uint32_t i) { return buffer[i]; }

    /*
     * Get a short from a buffer
     * This method is unsafe, the caller is responsible for performing a check
     */
    static uint16_t UnsafeGetShort(const uint8_t* buffer, uint32_t i) {
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
    static uint32_t UnsafeGetInt(const uint8_t* buffer, uint32_t i) {
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
    static bool IsValidEndianMarker(const uint8_t* data, bool* isLittleEndian) {
        // II indicates Intel (little endian) and MM indicates motorola (big endian).
        if (('I' != data[0] || 'I' != data[1]) && ('M' != data[0] || 'M' != data[1])) {
            return false;
        }

        *isLittleEndian = ('I' == data[0]);
        return true;
    }

    static uint16_t GetEndianShort(const uint8_t* data, bool littleEndian) {
        if (littleEndian) {
            return (data[1] << 8) | (data[0]);
        }

        return (data[0] << 8) | (data[1]);
    }

    static uint32_t GetEndianInt(const uint8_t* data, bool littleEndian) {
        if (littleEndian) {
            return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);
        }

        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
    }

    static SkPMColor PremultiplyARGBasRGBA(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
        if (a != 255) {
            r = SkMulDiv255Round(r, a);
            g = SkMulDiv255Round(g, a);
            b = SkMulDiv255Round(b, a);
        }

        return SkPackARGB_as_RGBA(a, r, g, b);
    }

    static SkPMColor PremultiplyARGBasBGRA(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
        if (a != 255) {
            r = SkMulDiv255Round(r, a);
            g = SkMulDiv255Round(g, a);
            b = SkMulDiv255Round(b, a);
        }

        return SkPackARGB_as_BGRA(a, r, g, b);
    }

    static bool IsRGBA(SkColorType colorType) {
#ifdef SK_PMCOLOR_IS_RGBA
        return (kBGRA_8888_SkColorType != colorType);
#else
        return (kRGBA_8888_SkColorType == colorType);
#endif
    }

    // Method for coverting to a 32 bit pixel.
    using PackColorProc = uint32_t (*)(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

    static PackColorProc ChoosePackColorProc(bool isPremul, SkColorType colorType) {
        bool isRGBA = IsRGBA(colorType);
        if (isPremul) {
            if (isRGBA) {
                return &PremultiplyARGBasRGBA;
            } else {
                return &PremultiplyARGBasBGRA;
            }
        } else {
            if (isRGBA) {
                return &SkPackARGB_as_RGBA;
            } else {
                return &SkPackARGB_as_BGRA;
            }
        }
    }

    static sk_sp<const SkData> GetEncodedData(const SkCodec* codec) {
        SkASSERT(codec);
        return codec->getEncodedData();
    }
};

#endif // SkCodecPriv_DEFINED
