/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorPriv.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformPriv.h"
#include "SkOpts.h"
#include "SkUtils.h"

static void clamp_to_alpha(uint32_t* pixels, int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = SkGetPackedA32(pixels[i]);
        uint8_t r = SkGetPackedR32(pixels[i]);
        uint8_t g = SkGetPackedG32(pixels[i]);
        uint8_t b = SkGetPackedB32(pixels[i]);
        pixels[i] = SkPackARGB32(a,
                                 SkTMin(a, r),
                                 SkTMin(a, g),
                                 SkTMin(a, b));
    }
}

class GammaEncodedPremulGM : public skiagm::GM {
public:
    GammaEncodedPremulGM(sk_sp<SkColorSpace> dst, sk_sp<SkColorSpace> src, const char* desc)
        : fDstSpace(dst)
        , fSrcSpace(src)
        , fXform(SkColorSpaceXform::New(src.get(), dst.get()))
        , fName(SkStringPrintf("gamma_encoded_premul_dst-v-src_%s", desc))
    {
        int i = 0;
        for (int r = 0; r < kColorSteps; r++) {
            for (int g = 0; g < kColorSteps; g++) {
                for (int b = 0; b < kColorSteps; b++) {
                    fColors[i++] = SkColorSetARGBInline(0xFF,
                                                        r * kColorScale,
                                                        g * kColorScale,
                                                        b * kColorScale);
                }
            }
        }

    }

protected:
    virtual SkISize onISize() override {
        return SkISize::Make(kAlphaMax, kNumColors * 2 * kStripeHeight);
    }

    SkString onShortName() override {
        return fName;
    }

    void onDraw(SkCanvas* canvas) override {
        if (canvas->imageInfo().isOpaque()) {
            return;
        }

        SkBitmap bitmap;
        SkImageInfo bitmapInfo = SkImageInfo::MakeN32Premul(kAlphaMax, 1,
                canvas->imageInfo().refColorSpace());
        bitmap.allocPixels(bitmapInfo);
        uint32_t* pixels = bitmap.getAddr32(0, 0);

        for (int i = 0; i < kNumColors; i++) {
            // Create an entire row of the same color, with the alpha from 0 to kAlphaMax.
            uint32_t row[kAlphaMax];
            sk_memset32(row, fColors[i], kAlphaMax);
            for (int a = 0; a < kAlphaMax; a++) {
                row[a] = (row[a] & 0x00FFFFFF) | (a << 24);
            }

            // Tranform row to dst, then premultiply.
            fXform->apply(select_xform_format(kN32_SkColorType), pixels,
                          SkColorSpaceXform::kBGRA_8888_ColorFormat, row, kAlphaMax,
                          kUnpremul_SkAlphaType);
            SkOpts::RGBA_to_rgbA(pixels, pixels, kAlphaMax);
            bitmap.notifyPixelsChanged();

            // Write the dst space premultiplied row to the canvas.
            for (int j = 0; j < kStripeHeight; j++) {
                canvas->drawBitmap(bitmap, 0, 2 * i * kStripeHeight + j);
            }

            // Premultiply, then transform the row to dst.
            SkOpts::RGBA_to_rgbA(pixels, row, kAlphaMax);
            fXform->apply(select_xform_format(kN32_SkColorType), pixels,
                          SkColorSpaceXform::kBGRA_8888_ColorFormat, pixels, kAlphaMax,
                          kUnpremul_SkAlphaType);
            clamp_to_alpha(pixels, kAlphaMax);
            bitmap.notifyPixelsChanged();

            // Write the src space premultiplied row to the canvas.
            for (int j = 0; j < kStripeHeight; j++) {
                canvas->drawBitmap(bitmap, 0, (2 * i + 1) * kStripeHeight + j);
            }
        }
    }

private:
    static constexpr int kColorSteps = 4;
    static constexpr int kNumColors = kColorSteps * kColorSteps * kColorSteps;
    static constexpr int kColorScale = 255 / (kColorSteps - 1);
    static constexpr int kStripeHeight = 10;
    static constexpr int kAlphaMax = 255;

    sk_sp<SkColorSpace>                fDstSpace;
    sk_sp<SkColorSpace>                fSrcSpace;
    std::unique_ptr<SkColorSpaceXform> fXform;
    SkString                           fName;
    SkColor                            fColors[kNumColors];

    typedef GM INHERITED;
};

DEF_GM(return new GammaEncodedPremulGM(SkColorSpace::MakeSRGB(),
        SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, SkColorSpace::kRec2020_Gamut),
        "toWideGamut");)
DEF_GM(return new GammaEncodedPremulGM(SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
        SkColorSpace::kRec2020_Gamut), SkColorSpace::MakeSRGB(), "fromWideGamut");)
DEF_GM(return new GammaEncodedPremulGM(SkColorSpace::MakeSRGB(),
        SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, SkColorSpace::kSRGB_Gamut),
        "toLinear");)
DEF_GM(return new GammaEncodedPremulGM(
        SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, SkColorSpace::kSRGB_Gamut),
        SkColorSpace::MakeSRGB(), "fromLinear");)
DEF_GM(return new GammaEncodedPremulGM(
        SkColorSpace::MakeRGB({ 1.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
        SkColorSpace::kSRGB_Gamut), SkColorSpace::MakeSRGB(), "from1.8");)
