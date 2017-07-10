/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedInfo_DEFINED
#define SkEncodedInfo_DEFINED

#include "SkImageInfo.h"

class SkColorSpace;

struct SkEncodedInfo {
public:

    enum Alpha {
        kOpaque_Alpha,
        kUnpremul_Alpha,

        // Each pixel is either fully opaque or fully transparent.
        // There is no difference between requesting kPremul or kUnpremul.
        kBinary_Alpha,
    };

    /*
     * We strive to make the number of components per pixel obvious through
     * our naming conventions.
     * Ex: kRGB has 3 components.  kRGBA has 4 components.
     *
     * This sometimes results in redundant Alpha and Color information.
     * Ex: kRGB images must also be kOpaque.
     */
    enum Color {
        // PNG, WBMP
        kGray_Color,

        // PNG
        kGrayAlpha_Color,

        // PNG, GIF, BMP
        kPalette_Color,

        // PNG, RAW
        kRGB_Color,
        kRGBA_Color,

        // BMP
        kBGR_Color,
        kBGRX_Color,
        kBGRA_Color,

        // JPEG, WEBP
        kYUV_Color,

        // WEBP
        kYUVA_Color,

        // JPEG
        // Photoshop actually writes inverted CMYK data into JPEGs, where zero
        // represents 100% ink coverage.  For this reason, we treat CMYK JPEGs
        // as having inverted CMYK.  libjpeg-turbo warns that this may break
        // other applications, but the CMYK JPEGs we see on the web expect to
        // be treated as inverted CMYK.
        kInvertedCMYK_Color,
        kYCCK_Color,
    };

    static SkEncodedInfo Make(Color color, Alpha alpha, int bitsPerComponent) {
        SkASSERT(1 == bitsPerComponent ||
                 2 == bitsPerComponent ||
                 4 == bitsPerComponent ||
                 8 == bitsPerComponent ||
                 16 == bitsPerComponent);

        switch (color) {
            case kGray_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                break;
            case kGrayAlpha_Color:
                SkASSERT(kOpaque_Alpha != alpha);
                break;
            case kPalette_Color:
                SkASSERT(16 != bitsPerComponent);
                break;
            case kRGB_Color:
            case kBGR_Color:
            case kBGRX_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                SkASSERT(bitsPerComponent >= 8);
                break;
            case kYUV_Color:
            case kInvertedCMYK_Color:
            case kYCCK_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                SkASSERT(8 == bitsPerComponent);
                break;
            case kRGBA_Color:
                SkASSERT(kOpaque_Alpha != alpha);
                SkASSERT(bitsPerComponent >= 8);
                break;
            case kBGRA_Color:
            case kYUVA_Color:
                SkASSERT(kOpaque_Alpha != alpha);
                SkASSERT(8 == bitsPerComponent);
                break;
            default:
                SkASSERT(false);
                break;
        }

        return SkEncodedInfo(color, alpha, bitsPerComponent);
    }

    /*
     * Returns an SkImageInfo with Skia color and alpha types that are the
     * closest possible match to the encoded info.
     */
    SkImageInfo makeImageInfo(int width, int height, sk_sp<SkColorSpace> colorSpace) const {
        switch (fColor) {
            case kGray_Color:
                SkASSERT(kOpaque_Alpha == fAlpha);
                return SkImageInfo::Make(width, height, kGray_8_SkColorType,
                                         kOpaque_SkAlphaType, colorSpace);
            case kGrayAlpha_Color:
                SkASSERT(kOpaque_Alpha != fAlpha);
                return SkImageInfo::Make(width, height, kN32_SkColorType,
                        kUnpremul_SkAlphaType, colorSpace);
            case kPalette_Color: {
                SkAlphaType alphaType = (kOpaque_Alpha == fAlpha) ? kOpaque_SkAlphaType :
                        kUnpremul_SkAlphaType;
                return SkImageInfo::Make(width, height, kN32_SkColorType,
                                         alphaType, colorSpace);
            }
            case kRGB_Color:
            case kBGR_Color:
            case kBGRX_Color:
            case kYUV_Color:
            case kInvertedCMYK_Color:
            case kYCCK_Color:
                SkASSERT(kOpaque_Alpha == fAlpha);
                return SkImageInfo::Make(width, height, kN32_SkColorType,
                                         kOpaque_SkAlphaType, colorSpace);
            case kRGBA_Color:
            case kBGRA_Color:
            case kYUVA_Color:
                SkASSERT(kOpaque_Alpha != fAlpha);
                return SkImageInfo::Make(width, height, kN32_SkColorType,
                                         kUnpremul_SkAlphaType, std::move(colorSpace));
            default:
                SkASSERT(false);
                return SkImageInfo::MakeUnknown();
        }
    }

    Color color() const { return fColor; }
    Alpha alpha() const { return fAlpha; }
    uint8_t bitsPerComponent() const { return fBitsPerComponent; }

    uint8_t bitsPerPixel() const {
        switch (fColor) {
            case kGray_Color:
                return fBitsPerComponent;
            case kGrayAlpha_Color:
                return 2 * fBitsPerComponent;
            case kPalette_Color:
                return fBitsPerComponent;
            case kRGB_Color:
            case kBGR_Color:
            case kYUV_Color:
                return 3 * fBitsPerComponent;
            case kRGBA_Color:
            case kBGRA_Color:
            case kBGRX_Color:
            case kYUVA_Color:
            case kInvertedCMYK_Color:
            case kYCCK_Color:
                return 4 * fBitsPerComponent;
            default:
                SkASSERT(false);
                return 0;
        }
    }

private:

    SkEncodedInfo(Color color, Alpha alpha, uint8_t bitsPerComponent)
        : fColor(color)
        , fAlpha(alpha)
        , fBitsPerComponent(bitsPerComponent)
    {}

    Color   fColor;
    Alpha   fAlpha;
    uint8_t fBitsPerComponent;
};

#endif
