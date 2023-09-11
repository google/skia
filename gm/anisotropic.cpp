/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

namespace skiagm {

// This GM exercises anisotropic image scaling.
class AnisotropicGM : public GM {
public:
    enum class Mode { kLinear, kMip, kAniso };

    AnisotropicGM(Mode mode) : fMode(mode) {
        switch (fMode) {
            case Mode::kLinear:
                fSampling = SkSamplingOptions(SkFilterMode::kLinear);
                break;
            case Mode::kMip:
                fSampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
                break;
            case Mode::kAniso:
                fSampling = SkSamplingOptions::Aniso(16);
                break;
        }
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override {
        SkString name("anisotropic_image_scale_");
        switch (fMode) {
            case Mode::kLinear:
                name += "linear";
                break;
            case Mode::kMip:
                name += "mip";
                break;
            case Mode::kAniso:
                name += "aniso";
                break;
        }
        return name;
    }

    SkISize getISize() override {
        return SkISize::Make(2*kImageSize + 3*kSpacer,
                             kNumVertImages*kImageSize + (kNumVertImages+1)*kSpacer);
    }

    // Create an image consisting of lines radiating from its center
    void onOnceBeforeDraw() override {
        constexpr int kNumLines = 100;
        constexpr SkScalar kAngleStep = 360.0f / kNumLines;
        constexpr int kInnerOffset = 10;

        auto info = SkImageInfo::MakeN32(kImageSize, kImageSize, kOpaque_SkAlphaType);
        auto surf = SkSurfaces::Raster(info);
        auto canvas = surf->getCanvas();

        canvas->clear(SK_ColorWHITE);

        SkPaint p;
        p.setAntiAlias(true);

        SkScalar angle = 0.0f, sin, cos;

        canvas->translate(kImageSize/2.0f, kImageSize/2.0f);
        for (int i = 0; i < kNumLines; ++i, angle += kAngleStep) {
            sin = SkScalarSin(angle);
            cos = SkScalarCos(angle);
            canvas->drawLine(cos * kInnerOffset, sin * kInnerOffset,
                             cos * kImageSize/2, sin * kImageSize/2, p);
        }
        fImage = surf->makeImageSnapshot();
    }

    void draw(SkCanvas* canvas, int x, int y, int xSize, int ySize) {
        SkRect r = SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
                                    SkIntToScalar(xSize), SkIntToScalar(ySize));
        canvas->drawImageRect(fImage, r, fSampling);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar gScales[] = { 0.9f, 0.8f, 0.75f, 0.6f, 0.5f, 0.4f, 0.25f, 0.2f, 0.1f };

        SkASSERT(kNumVertImages-1 == (int)std::size(gScales)/2);

        // Minimize vertically
        for (int i = 0; i < (int)std::size(gScales); ++i) {
            int height = SkScalarFloorToInt(fImage->height() * gScales[i]);

            int yOff;
            if (i <= (int)std::size(gScales)/2) {
                yOff = kSpacer + i * (fImage->height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                yOff = (std::size(gScales) - i) * (fImage->height() + kSpacer) - height;
            }

            this->draw(canvas, kSpacer, yOff, fImage->width(), height);
        }

        // Minimize horizontally
        for (int i = 0; i < (int)std::size(gScales); ++i) {
            int width = SkScalarFloorToInt(fImage->width() * gScales[i]);

            int xOff, yOff;
            if (i <= (int)std::size(gScales)/2) {
                xOff = fImage->width() + 2*kSpacer;
                yOff = kSpacer + i * (fImage->height() + kSpacer);
            } else {
                // Position the more highly squashed images with their less squashed counterparts
                xOff = fImage->width() + 2*kSpacer + fImage->width() - width;
                yOff = kSpacer + (std::size(gScales) - i - 1) * (fImage->height() + kSpacer);
            }

            this->draw(canvas, xOff, yOff, width, fImage->height());
        }
    }

private:
    inline static constexpr int kImageSize     = 256;
    inline static constexpr int kSpacer        = 10;
    inline static constexpr int kNumVertImages = 5;

    sk_sp<SkImage>    fImage;
    SkSamplingOptions fSampling;
    Mode              fMode;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnisotropicGM(AnisotropicGM::Mode::kLinear);)
DEF_GM(return new AnisotropicGM(AnisotropicGM::Mode::kMip);)
DEF_GM(return new AnisotropicGM(AnisotropicGM::Mode::kAniso);)

//////////////////////////////////////////////////////////////////////////////

class AnisoMipsGM : public GM {
public:
    AnisoMipsGM() = default;

protected:
    SkString getName() const override { return SkString("anisomips"); }

    SkISize getISize() override { return SkISize::Make(520, 260); }

    sk_sp<SkImage> updateImage(SkSurface* surf, SkColor color) {
        surf->getCanvas()->clear(color);
        SkPaint paint;
        paint.setColor(~color | 0xFF000000);
        surf->getCanvas()->drawRect(SkRect::MakeLTRB(surf->width() *2/5.f,
                                                     surf->height()*2/5.f,
                                                     surf->width() *3/5.f,
                                                     surf->height()*3/5.f),
                                    paint);
        return surf->makeImageSnapshot()->withDefaultMipmaps();
    }

    void onDraw(SkCanvas* canvas) override {
        auto ct = canvas->imageInfo().colorType() == kUnknown_SkColorType
                          ? kRGBA_8888_SkColorType
                          : canvas->imageInfo().colorType();
        auto ii = SkImageInfo::Make(kImageSize,
                                    kImageSize,
                                    ct,
                                    kPremul_SkAlphaType,
                                    canvas->imageInfo().refColorSpace());
        // In GPU mode we want a surface that is created with mipmaps to ensure that we exercise the
        // case where the SkSurface and SkImage share a texture. If the surface texture isn't
        // created with MIPs then asking for a mipmapped image will cause a copy to a mipped
        // texture.
        sk_sp<SkSurface> surface;
        if (auto rc = canvas->recordingContext()) {
            surface = SkSurfaces::RenderTarget(rc,
                                               skgpu::Budgeted::kYes,
                                               ii,
                                               /* sampleCount= */ 1,
                                               kTopLeft_GrSurfaceOrigin,
                                               /*surfaceProps=*/nullptr,
                                               /*shouldCreateWithMips=*/true);
            if (!surface) {
                // We could be in an abandoned context situation.
                return;
            }
        } else {
            surface = canvas->makeSurface(ii);
            if (!surface) {  // could be a recording canvas.
                surface = SkSurfaces::Raster(ii);
            }
        }

        static constexpr float kScales[] = {1.f, 0.5f, 0.25f, 0.125f};
        SkColor kColors[] = {0xFFF0F0F0, SK_ColorBLUE, SK_ColorGREEN, SK_ColorRED};
        static const SkSamplingOptions kSampling = SkSamplingOptions::Aniso(16);

        for (bool shader : {false, true}) {
            int c = 0;
            canvas->save();
            for (float sy : kScales) {
                canvas->save();
                for (float sx : kScales) {
                    canvas->save();
                    canvas->scale(sx, sy);
                    auto image = this->updateImage(surface.get(), kColors[c]);
                    if (shader) {
                        SkPaint paint;
                        paint.setShader(image->makeShader(kSampling));
                        canvas->drawRect(SkRect::Make(image->dimensions()), paint);
                    } else {
                        canvas->drawImage(image, 0, 0, kSampling);
                    }
                    canvas->restore();
                    canvas->translate(ii.width() * sx + kPad, 0);
                    c = (c + 1) % std::size(kColors);
                }
                canvas->restore();
                canvas->translate(0, ii.width() * sy + kPad);
            }
            canvas->restore();
            for (float sx : kScales) {
                canvas->translate(ii.width() * sx + kPad, 0);
            }
        }
    }

private:
    inline static constexpr int kImageSize = 128;
    inline static constexpr int kPad = 5;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new AnisoMipsGM();)

}  // namespace skiagm
