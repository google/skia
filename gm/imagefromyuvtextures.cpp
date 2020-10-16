/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAIndex.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkYUVMath.h"
#include "tools/Resources.h"
#include "tools/gpu/YUVUtils.h"

using sk_gpu_test::YUVABackendReleaseContext;

class GrRenderTargetContext;

namespace skiagm {
class ImageFromYUVTextures : public GpuGM {
public:
    ImageFromYUVTextures() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("image_from_yuv_textures");
    }

    SkISize onISize() override { return {1420, 610}; }

    static SkBitmap CreateBmpAndPlanes(const char* name, SkBitmap yuvaBmps[4]) {
        SkBitmap bmp;
        if (!GetResourceAsBitmap(name, &bmp)) {
            return {};
        }
        auto ii = SkImageInfo::Make(bmp.dimensions(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        SkBitmap rgbaBmp;
        rgbaBmp.allocPixels(ii);
        bmp.readPixels(rgbaBmp.pixmap(), 0, 0);

        SkImageInfo yaInfo = SkImageInfo::Make(rgbaBmp.dimensions(), kAlpha_8_SkColorType,
                                               kUnpremul_SkAlphaType);
        yuvaBmps[0].allocPixels(yaInfo);
        SkISize uvSize = {rgbaBmp.width()/2, rgbaBmp.height()/2};
        SkImageInfo uvInfo = SkImageInfo::Make(uvSize, kAlpha_8_SkColorType, kUnpremul_SkAlphaType);
        yuvaBmps[1].allocPixels(uvInfo);
        yuvaBmps[2].allocPixels(uvInfo);
        yuvaBmps[3].allocPixels(yaInfo);

        unsigned char* yuvPixels[] = {
                static_cast<unsigned char*>(yuvaBmps[0].getPixels()),
                static_cast<unsigned char*>(yuvaBmps[1].getPixels()),
                static_cast<unsigned char*>(yuvaBmps[2].getPixels()),
                static_cast<unsigned char*>(yuvaBmps[3].getPixels()),
        };

        float m[20];
        SkColorMatrix_RGB2YUV(kJPEG_SkYUVColorSpace, m);
        // Here we encode using the kJPEG_SkYUVColorSpace (i.e., full-swing Rec 601) even though
        // we will draw it with all the supported yuv color spaces when converted back to RGB
        for (int j = 0; j < yaInfo.height(); ++j) {
            for (int i = 0; i < yaInfo.width(); ++i) {
                auto rgba = *rgbaBmp.getAddr32(i, j);
                auto r = (rgba & 0x000000ff) >>  0;
                auto g = (rgba & 0x0000ff00) >>  8;
                auto b = (rgba & 0x00ff0000) >> 16;
                auto a = (rgba & 0xff000000) >> 24;
                yuvPixels[0][j*yaInfo.width() + i] = SkToU8(
                        sk_float_round2int(m[0]*r + m[1]*g + m[2]*b + m[3]*a + 255*m[4]));
                yuvPixels[3][j*yaInfo.width() + i] = SkToU8(sk_float_round2int(
                        m[15]*r + m[16]*g + m[17]*b + m[18]*a + 255*m[19]));
            }
        }
        for (int j = 0; j < uvInfo.height(); ++j) {
            for (int i = 0; i < uvInfo.width(); ++i) {
                // Average together 4 pixels of RGB.
                int rgba[] = {0, 0, 0, 0};
                for (int y = 0; y < 2; ++y) {
                    for (int x = 0; x < 2; ++x) {
                        auto src = *rgbaBmp.getAddr32(2 * i + x, 2 * j + y);
                        rgba[0] += (src & 0x000000ff) >> 0;
                        rgba[1] += (src & 0x0000ff00) >> 8;
                        rgba[2] += (src & 0x00ff0000) >> 16;
                        rgba[3] += (src & 0xff000000) >> 24;
                    }
                }
                for (int c = 0; c < 4; ++c) {
                    rgba[c] /= 4;
                }
                int uvIndex = j*uvInfo.width() + i;
                yuvPixels[1][uvIndex] = SkToU8(sk_float_round2int(
                        m[5]*rgba[0] + m[6]*rgba[1] + m[7]*rgba[2] + m[8]*rgba[3] + 255*m[9]));
                yuvPixels[2][uvIndex] = SkToU8(sk_float_round2int(
                        m[10]*rgba[0] + m[11]*rgba[1] + m[12]*rgba[2] + m[13]*rgba[3] + 255*m[14]));
            }
        }
        return rgbaBmp;
    }

    static bool CreateYUVBackendTextures(GrDirectContext* context, SkBitmap bmps[4],
                                         SkYUVAIndex indices[4],
                                         YUVABackendReleaseContext* beContext) {
        for (int i = 0; i < 4; ++i) {
            GrBackendTexture tmp = context->createBackendTexture(
                                        bmps[i].pixmap(), GrRenderable::kNo, GrProtected::kNo,
                                        YUVABackendReleaseContext::CreationCompleteProc(i),
                                        beContext);
            if (!tmp.isValid()) {
                return false;
            }

            beContext->set(i, tmp);
        }

        for (int i = 0; i < 4; ++i) {
            auto chanMask = beContext->beTexture(i).getBackendFormat().channelMask();
            // We expect the single channel bitmaps to produce single channel textures.
            SkASSERT(chanMask && SkIsPow2(chanMask));
            if (chanMask & kGray_SkColorChannelFlag) {
                indices[i].fChannel = SkColorChannel::kR;
            } else {
                indices[i].fChannel = static_cast<SkColorChannel>(31 - SkCLZ(chanMask));
            }
            indices[i].fIndex = i;
        }

        return true;
    }

    sk_sp<SkImage> makeYUVAImage(GrDirectContext* context) {
        auto releaseContext = new YUVABackendReleaseContext(context);
        SkYUVAIndex indices[4];

        if (!CreateYUVBackendTextures(context, fYUVABmps, indices, releaseContext)) {
            YUVABackendReleaseContext::Unwind(context, releaseContext, false);
            return nullptr;
        }

        return SkImage::MakeFromYUVATextures(context,
                                             kJPEG_SkYUVColorSpace,
                                             releaseContext->beTextures(),
                                             indices,
                                             fRGBABmp.dimensions(),
                                             kTopLeft_GrSurfaceOrigin,
                                             nullptr,
                                             YUVABackendReleaseContext::Release,
                                             releaseContext);
    }

    sk_sp<SkImage> createReferenceImage(GrDirectContext* dContext) {
        auto resultInfo = SkImageInfo::Make(fRGBABmp.dimensions(),
                                            kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType);
        auto resultSurface = SkSurface::MakeRenderTarget(dContext,
                                                         SkBudgeted::kYes,
                                                         resultInfo,
                                                         1,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         nullptr);
        if (!resultSurface) {
            return nullptr;
        }

        auto planeReleaseContext = new YUVABackendReleaseContext(dContext);
        SkYUVAIndex indices[4];

        if (!CreateYUVBackendTextures(dContext, fYUVABmps, indices, planeReleaseContext)) {
            YUVABackendReleaseContext::Unwind(dContext, planeReleaseContext, false);
            return nullptr;
        }

        auto tmp = SkImage::MakeFromYUVATextures(dContext,
                                                 kJPEG_SkYUVColorSpace,
                                                 planeReleaseContext->beTextures(),
                                                 indices,
                                                 fRGBABmp.dimensions(),
                                                 kTopLeft_GrSurfaceOrigin,
                                                 nullptr);
        if (!tmp) {
            YUVABackendReleaseContext::Unwind(dContext, planeReleaseContext, false);
            return nullptr;
        }
        resultSurface->getCanvas()->drawImage(std::move(tmp), 0, 0);
        YUVABackendReleaseContext::Unwind(dContext, planeReleaseContext, true);
        return resultSurface->makeImageSnapshot();
    }

    DrawResult onGpuSetup(GrDirectContext* context, SkString* errorMsg) override {
        if (!context || context->abandoned()) {
            return DrawResult::kSkip;
        }

        fRGBABmp = CreateBmpAndPlanes("images/mandrill_32.png", fYUVABmps);

        // We make a version of this image for each draw because, if any draw flattens it to
        // RGBA, then all subsequent draws would use the RGBA texture.
        for (int i = 0; i < kNumImages; ++i) {
            fYUVAImages[i] = this->makeYUVAImage(context);
            if (!fYUVAImages[i]) {
                *errorMsg = "Couldn't create src YUVA image.";
                return DrawResult::kFail;
            }
        }

        fReferenceImage = this->createReferenceImage(context);
        if (!fReferenceImage) {
            *errorMsg = "Couldn't create reference YUVA image.";
            return DrawResult::kFail;
        }

        // Some backends (e.g., Vulkan) require all work be completed for backend textures
        // before they are deleted. Since we don't know when we'll next have access to a
        // direct context, flush all the work now.
        context->flush();
        context->submit(true);

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        for (sk_sp<SkImage>& image : fYUVAImages) {
            image.reset();
        }
        fReferenceImage.reset();
    }

    SkImage* getYUVAImage(int index) {
        SkASSERT(index >= 0 && index < kNumImages);
        return fYUVAImages[index].get();
    }

    void onDraw(GrRecordingContext*, GrRenderTargetContext*, SkCanvas* canvas) override {
        auto draw_image = [canvas](SkImage* image, SkFilterQuality fq) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            SkPaint paint;
            paint.setFilterQuality(fq);
            canvas->drawImage(image, 0, 0, &paint);
            return {SkIntToScalar(image->width()), SkIntToScalar(image->height())};
        };

        auto draw_image_rect = [canvas](SkImage* image, SkFilterQuality fq) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            SkPaint paint;
            paint.setFilterQuality(fq);
            auto subset = SkRect::Make(image->dimensions());
            subset.inset(subset.width() * .05f, subset.height() * .1f);
            auto dst = SkRect::MakeWH(subset.width(), subset.height());
            canvas->drawImageRect(image, subset, dst, &paint);
            return {dst.width(), dst.height()};
        };

        auto draw_image_shader = [canvas](SkImage* image, SkFilterQuality fq) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            SkMatrix m;
            m.setRotate(45, image->width()/2.f, image->height()/2.f);
            auto shader = image->makeShader(SkTileMode::kMirror, SkTileMode::kDecal, m);
            SkPaint paint;
            paint.setFilterQuality(fq);
            paint.setShader(std::move(shader));
            auto rect = SkRect::MakeWH(image->width() * 1.3f, image->height());
            canvas->drawRect(rect, paint);
            return {rect.width(), rect.height()};
        };

        canvas->translate(kPad, kPad);
        int imageIndex = 0;
        using DrawSig = SkSize(SkImage* image, SkFilterQuality fq);
        using DF = std::function<DrawSig>;
        for (const auto& draw : {DF(draw_image), DF(draw_image_rect), DF(draw_image_shader)}) {
            for (auto scale : {1.f, 4.f, 0.75f}) {
                SkScalar h = 0;
                canvas->save();
                for (auto fq : {kNone_SkFilterQuality, kLow_SkFilterQuality,
                                kMedium_SkFilterQuality, kHigh_SkFilterQuality}) {
                    canvas->save();
                        canvas->scale(scale, scale);
                        auto s1 = draw(this->getYUVAImage(imageIndex++), fq);
                    canvas->restore();
                    canvas->translate(kPad + SkScalarCeilToScalar(scale*s1.width()), 0);
                    canvas->save();
                        canvas->scale(scale, scale);
                        auto s2 = draw(fReferenceImage.get(), fq);
                    canvas->restore();
                    canvas->translate(kPad + SkScalarCeilToScalar(scale*s2.width()), 0);
                    h = std::max({h, s1.height(), s2.height()});
                }
                canvas->restore();
                canvas->translate(0, kPad + SkScalarCeilToScalar(scale*h));
            }
        }
     }

private:
    SkBitmap fRGBABmp; // TODO: oddly, it looks like this could just be an SkISize
    SkBitmap fYUVABmps[4];

    // 3 draws x 3 scales x 4 filter qualities
    static constexpr int kNumImages = 3 * 3 * 4;
    sk_sp<SkImage> fYUVAImages[kNumImages];
    sk_sp<SkImage> fReferenceImage;

    static constexpr SkScalar kPad = 10.0f;

    using INHERITED = GM;
};

DEF_GM(return new ImageFromYUVTextures;)
}  // namespace skiagm
