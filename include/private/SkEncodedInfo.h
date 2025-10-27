/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedInfo_DEFINED
#define SkEncodedInfo_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/SkHdrMetadata.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

struct SkEncodedInfo {
public:
    class ICCProfile {
    public:
        static std::unique_ptr<ICCProfile> Make(sk_sp<SkData>);
        static std::unique_ptr<ICCProfile> Make(const skcms_ICCProfile&);

        const skcms_ICCProfile* profile() const { return &fProfile; }
        sk_sp<SkData> data() const { return fData; }
    private:
        ICCProfile(const skcms_ICCProfile&, sk_sp<SkData> = nullptr);

        skcms_ICCProfile fProfile;
        sk_sp<SkData>    fData;
    };

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

        // PNG with Skia-specific sBIT
        // Like kGrayAlpha, except this expects to be treated as
        // kAlpha_8_SkColorType, which ignores the gray component. If
        // decoded to full color (e.g. kN32), the gray component is respected
        // (so it can share code with kGrayAlpha).
        kXAlpha_Color,

        // PNG
        // 565 images may be encoded to PNG by specifying the number of
        // significant bits for each channel.  This is a strange 565
        // representation because the image is still encoded with 8 bits per
        // component.
        k565_Color,

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

    static SkEncodedInfo Make(int width, int height, Color color, Alpha alpha,
            int bitsPerComponent) {
        return Make(width, height, color, alpha, bitsPerComponent, nullptr);
    }

    static SkEncodedInfo Make(int width, int height, Color color,
            Alpha alpha, int bitsPerComponent, std::unique_ptr<ICCProfile> profile) {
        return Make(width, height, color, alpha, /*bitsPerComponent*/ bitsPerComponent,
                std::move(profile), /*colorDepth*/ bitsPerComponent);
    }

    static SkEncodedInfo Make(int width, int height, Color color,
            Alpha alpha, int bitsPerComponent, std::unique_ptr<ICCProfile> profile,
            int colorDepth) {
        return Make(width, height, color, alpha, bitsPerComponent, colorDepth, std::move(profile),
                    skhdr::Metadata::MakeEmpty());
    }

    static SkEncodedInfo Make(int width, int height, Color color,
            Alpha alpha, int bitsPerComponent, int colorDepth, std::unique_ptr<ICCProfile> profile,
            const skhdr::Metadata& hdrMetadata) {
        SkASSERT(1 == bitsPerComponent ||
                 2 == bitsPerComponent ||
                 4 == bitsPerComponent ||
                 8 == bitsPerComponent ||
                 16 == bitsPerComponent);
        VerifyColor(color, alpha, bitsPerComponent);
        return SkEncodedInfo(width,
                             height,
                             color,
                             alpha,
                             SkToU8(bitsPerComponent),
                             SkToU8(colorDepth),
                             std::move(profile),
                             hdrMetadata);
    }

    /*
     * Returns a recommended SkImageInfo.
     *
     * TODO: Leave this up to the client.
     */
    SkImageInfo makeImageInfo() const {
        auto ct =  kGray_Color == fColor ? kGray_8_SkColorType   :
                 kXAlpha_Color == fColor ? kAlpha_8_SkColorType  :
                    k565_Color == fColor ? kRGB_565_SkColorType  :
                                           kN32_SkColorType      ;
        auto alpha = kOpaque_Alpha == fAlpha ? kOpaque_SkAlphaType
                                             : kUnpremul_SkAlphaType;
        sk_sp<SkColorSpace> cs = fProfile ? SkColorSpace::Make(*fProfile->profile())
                                          : nullptr;
        if (!cs) {
            cs = SkColorSpace::MakeSRGB();
        }
        return SkImageInfo::Make(fWidth, fHeight, ct, alpha, std::move(cs));
    }

    int   width() const { return fWidth;  }
    int  height() const { return fHeight; }
    Color color() const { return fColor;  }
    Alpha alpha() const { return fAlpha;  }
    bool opaque() const { return fAlpha == kOpaque_Alpha; }
    const skcms_ICCProfile* profile() const {
        if (!fProfile) return nullptr;
        return fProfile->profile();
    }
    sk_sp<SkData> profileData() const {
        if (!fProfile) return nullptr;
        return fProfile->data();
    }

    uint8_t bitsPerComponent() const { return fBitsPerComponent; }

    uint8_t bitsPerPixel() const {
        switch (fColor) {
            case kGray_Color:
                return fBitsPerComponent;
            case kXAlpha_Color:
            case kGrayAlpha_Color:
                return 2 * fBitsPerComponent;
            case kPalette_Color:
                return fBitsPerComponent;
            case kRGB_Color:
            case kBGR_Color:
            case kYUV_Color:
            case k565_Color:
                return 3 * fBitsPerComponent;
            case kRGBA_Color:
            case kBGRA_Color:
            case kBGRX_Color:
            case kYUVA_Color:
            case kInvertedCMYK_Color:
            case kYCCK_Color:
                return 4 * fBitsPerComponent;
        }
        SkASSERT(false);
        return 0;
    }

    SkEncodedInfo(const SkEncodedInfo& orig) = delete;
    SkEncodedInfo& operator=(const SkEncodedInfo&) = delete;

    SkEncodedInfo(SkEncodedInfo&& orig) = default;
    SkEncodedInfo& operator=(SkEncodedInfo&&) = default;

    // Explicit copy method, to avoid accidental copying.
    SkEncodedInfo copy() const {
        return SkEncodedInfo(fWidth,
                             fHeight,
                             fColor,
                             fAlpha,
                             fBitsPerComponent,
                             fColorDepth,
                             fProfile ? std::make_unique<const ICCProfile>(*fProfile) : nullptr,
                             fHdrMetadata);
    }

    // Return number of bits of R/G/B channel
    uint8_t getColorDepth() const {
        return fColorDepth;
    }

    // Return the HDR metadata associated with this image. Note that even SDR images can include
    // HDR metadata (e.g, indicating how to inverse tone map when displayed on an HDR display).
    const skhdr::Metadata& getHdrMetadata() const {
        return fHdrMetadata;
    }

private:
    SkEncodedInfo(int width,
                  int height,
                  Color color,
                  Alpha alpha,
                  uint8_t bitsPerComponent,
                  uint8_t colorDepth,
                  std::unique_ptr<const ICCProfile> profile,
                  const skhdr::Metadata& hdrMetadata)
            : fWidth(width)
            , fHeight(height)
            , fColor(color)
            , fAlpha(alpha)
            , fBitsPerComponent(bitsPerComponent)
            , fColorDepth(colorDepth)
            , fProfile(std::move(profile))
            , fHdrMetadata(hdrMetadata) {}

    static void VerifyColor(Color color, Alpha alpha, int bitsPerComponent) {
        // Avoid `-Wunused-parameter` warnings on non-debug builds.
        std::ignore = alpha;
        std::ignore = bitsPerComponent;

        switch (color) {
            case kGray_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                return;
            case kGrayAlpha_Color:
                SkASSERT(kOpaque_Alpha != alpha);
                return;
            case kPalette_Color:
                SkASSERT(16 != bitsPerComponent);
                return;
            case kRGB_Color:
            case kBGR_Color:
            case kBGRX_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                SkASSERT(bitsPerComponent >= 8);
                return;
            case kYUV_Color:
            case kInvertedCMYK_Color:
            case kYCCK_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                SkASSERT(8 == bitsPerComponent);
                return;
            case kRGBA_Color:
                SkASSERT(bitsPerComponent >= 8);
                return;
            case kBGRA_Color:
            case kYUVA_Color:
                SkASSERT(8 == bitsPerComponent);
                return;
            case kXAlpha_Color:
                SkASSERT(kUnpremul_Alpha == alpha);
                SkASSERT(8 == bitsPerComponent);
                return;
            case k565_Color:
                SkASSERT(kOpaque_Alpha == alpha);
                SkASSERT(8 == bitsPerComponent);
                return;
        }
        SkASSERT(false);  // Unrecognized `color` enum value.
    }

    int                               fWidth;
    int                               fHeight;
    Color                                   fColor;
    Alpha                             fAlpha;
    uint8_t                           fBitsPerComponent;
    uint8_t                           fColorDepth;
    std::unique_ptr<const ICCProfile> fProfile;
    skhdr::Metadata                   fHdrMetadata;
};

#endif
