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
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkYUVMath.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/gpu/YUVUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Surface.h"
#endif

namespace skiagm {
class ImageFromYUV : public GM {
public:
    enum class Source {
        kTextures,
        kImages,
    };

    ImageFromYUV(Source source) : fSource(source) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override {
        switch (fSource) {
            case Source::kTextures: return SkString("image_from_yuv_textures");
            case Source::kImages:   return SkString("image_from_yuv_images");
        }
        SkUNREACHABLE;
    }

    SkISize getISize() override { return {1950, 800}; }

    static std::unique_ptr<sk_gpu_test::LazyYUVImage> CreatePlanes(const char* name) {
        SkBitmap bmp;
        if (!ToolUtils::GetResourceAsBitmap(name, &bmp)) {
            return {};
        }
        if (bmp.colorType() != kRGBA_8888_SkColorType) {
            auto info = bmp.info().makeColorType(kRGBA_8888_SkColorType);
            SkBitmap copy;
            copy.allocPixels(info);
            SkAssertResult(bmp.readPixels(copy.pixmap()));
            bmp = copy;
        }
        SkYUVAPixmapInfo pixmapInfo({bmp.dimensions(),
                                     SkYUVAInfo::PlaneConfig::kY_U_V_A,
                                     SkYUVAInfo::Subsampling::k420,
                                     kJPEG_Full_SkYUVColorSpace},
                                    SkYUVAPixmapInfo::DataType::kUnorm8,
                                    nullptr);
        auto pixmaps = SkYUVAPixmaps::Allocate(pixmapInfo);

        unsigned char* yuvPixels[] = {
                static_cast<unsigned char*>(pixmaps.planes()[0].writable_addr()),
                static_cast<unsigned char*>(pixmaps.planes()[1].writable_addr()),
                static_cast<unsigned char*>(pixmaps.planes()[2].writable_addr()),
                static_cast<unsigned char*>(pixmaps.planes()[3].writable_addr()),
        };

        float m[20];
        SkColorMatrix_RGB2YUV(pixmaps.yuvaInfo().yuvColorSpace(), m);
        // Here we encode using the kJPEG_SkYUVColorSpace (i.e., full-swing Rec 601) even though
        // we will draw it with all the supported yuv color spaces when converted back to RGB
        for (int j = 0; j < pixmaps.planes()[0].height(); ++j) {
            for (int i = 0; i < pixmaps.planes()[0].width(); ++i) {
                auto rgba = *bmp.getAddr32(i, j);
                auto r = (rgba & 0x000000ff) >>  0;
                auto g = (rgba & 0x0000ff00) >>  8;
                auto b = (rgba & 0x00ff0000) >> 16;
                auto a = (rgba & 0xff000000) >> 24;
                yuvPixels[0][j*pixmaps.planes()[0].width() + i] = SkToU8(
                        sk_float_round2int(m[0]*r + m[1]*g + m[2]*b + m[3]*a + 255*m[4]));
                yuvPixels[3][j*pixmaps.planes()[0].width() + i] = SkToU8(sk_float_round2int(
                        m[15]*r + m[16]*g + m[17]*b + m[18]*a + 255*m[19]));
            }
        }
        for (int j = 0; j < pixmaps.planes()[1].height(); ++j) {
            for (int i = 0; i < pixmaps.planes()[1].width(); ++i) {
                // Average together 4 pixels of RGB.
                int rgba[] = {0, 0, 0, 0};
                int denom = 0;
                int ylimit = std::min(2*j + 2, pixmaps.planes()[0].height());
                int xlimit = std::min(2*i + 2, pixmaps.planes()[0].width());
                for (int y = 2*j; y < ylimit; ++y) {
                    for (int x = 2*i; x < xlimit; ++x) {
                        auto src = *bmp.getAddr32(x, y);
                        rgba[0] += (src & 0x000000ff) >> 0;
                        rgba[1] += (src & 0x0000ff00) >> 8;
                        rgba[2] += (src & 0x00ff0000) >> 16;
                        rgba[3] += (src & 0xff000000) >> 24;
                        ++denom;
                    }
                }
                for (int c = 0; c < 4; ++c) {
                    rgba[c] /= denom;
                }
                int uvIndex = j*pixmaps.planes()[1].width() + i;
                yuvPixels[1][uvIndex] = SkToU8(sk_float_round2int(
                        m[5]*rgba[0] + m[6]*rgba[1] + m[7]*rgba[2] + m[8]*rgba[3] + 255*m[9]));
                yuvPixels[2][uvIndex] = SkToU8(sk_float_round2int(
                        m[10]*rgba[0] + m[11]*rgba[1] + m[12]*rgba[2] + m[13]*rgba[3] + 255*m[14]));
            }
        }
        return sk_gpu_test::LazyYUVImage::Make(std::move(pixmaps), skgpu::Mipmapped::kYes);
    }

    sk_sp<SkImage> makeYUVAImage(GrDirectContext* context, skgpu::graphite::Recorder* recorder) {
        SkASSERT(!(SkToBool(context) && SkToBool(recorder)));
        sk_gpu_test::LazyYUVImage::Type type;
        switch (fSource) {
            case Source::kTextures: type = sk_gpu_test::LazyYUVImage::Type::kFromTextures; break;
            case Source::kImages:   type = sk_gpu_test::LazyYUVImage::Type::kFromImages;   break;
        }
#if defined(SK_GANESH)
        if (context) {
            return fLazyYUVImage->refImage(context, type);
        }
#endif
#if defined(SK_GRAPHITE)
        if (recorder) {
            return fLazyYUVImage->refImage(recorder, type);
        }
#endif
        return nullptr;
    }

    sk_sp<SkImage> createReferenceImage(GrDirectContext* dContext,
                                        skgpu::graphite::Recorder* recorder) {
        auto planarImage = this->makeYUVAImage(dContext, recorder);
        if (!planarImage) {
            return nullptr;
        }

        auto resultInfo = SkImageInfo::Make(fLazyYUVImage->dimensions(),
                                            kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType);
        sk_sp<SkSurface> resultSurface;
#if defined(SK_GANESH)
        if (dContext) {
            resultSurface = SkSurfaces::RenderTarget(dContext,
                                                     skgpu::Budgeted::kYes,
                                                     resultInfo,
                                                     1,
                                                     kTopLeft_GrSurfaceOrigin,
                                                     nullptr,
                                                     /*shouldCreateWithMips=*/true);
        }
#endif
#if defined(SK_GRAPHITE)
        if (recorder) {
            resultSurface = SkSurfaces::RenderTarget(recorder, resultInfo, skgpu::Mipmapped::kYes);
        }
#endif
        if (!resultSurface) {
            return nullptr;
        }

        resultSurface->getCanvas()->drawImage(std::move(planarImage), 0, 0);
        return resultSurface->makeImageSnapshot();
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg, GraphiteTestContext*) override {
        auto* recorder = canvas->recorder();

#if defined(SK_GANESH)
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!recorder && (!dContext || dContext->abandoned())) {
            *errorMsg = "DirectContext or graphite::Recorder required to create YUV images";
            return DrawResult::kSkip;
        }

        if (dContext && !dContext->priv().caps()->mipmapSupport()) {
            return DrawResult::kSkip;
        }

        if (fSource == Source::kImages && dContext) {
            *errorMsg = "YUV Image from SkImage planes not supported with Ganesh.";
            return DrawResult::kSkip;
        }
#else
        constexpr GrDirectContext* dContext = nullptr;
#endif

        if (!fLazyYUVImage) {
            fLazyYUVImage = CreatePlanes("images/mandrill_128.png");
        }

        // We make a version of this image for each draw because, if any draw flattens it to
        // RGBA, then all subsequent draws would use the RGBA texture.
        for (int i = 0; i < kNumImages; ++i) {
            fYUVAImages[i] = this->makeYUVAImage(dContext, recorder);
            if (!fYUVAImages[i]) {
                *errorMsg = "Couldn't create src YUVA image.";
                return DrawResult::kFail;
            }
        }

        fReferenceImage = this->createReferenceImage(dContext, recorder);
        if (!fReferenceImage) {
            *errorMsg = "Couldn't create reference YUVA image.";
            return DrawResult::kFail;
        }

#if defined(SK_GANESH)
        if (dContext) {
            // Some backends (e.g., Vulkan) require all work be completed for backend textures
            // before they are deleted. Since we don't know when we'll next have access to a
            // direct context, flush all the work now.
            dContext->flush();
            dContext->submit(GrSyncCpu::kYes);
        }
#endif

        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        fLazyYUVImage.reset();
        for (sk_sp<SkImage>& image : fYUVAImages) {
            image.reset();
        }
        fReferenceImage.reset();
    }

    SkImage* getYUVAImage(int index) {
        SkASSERT(index >= 0 && index < kNumImages);
        return fYUVAImages[index].get();
    }

    void onDraw(SkCanvas* canvas) override {
        auto draw_image = [canvas](SkImage* image, const SkSamplingOptions& sampling) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            canvas->drawImage(image, 0, 0, sampling, nullptr);
            return {SkIntToScalar(image->width()), SkIntToScalar(image->height())};
        };

        auto draw_image_rect = [canvas](SkImage* image,
                                        const SkSamplingOptions& sampling) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            auto subset = SkRect::Make(image->dimensions());
            subset.inset(subset.width() * .05f, subset.height() * .1f);
            auto dst = SkRect::MakeWH(subset.width(), subset.height());
            canvas->drawImageRect(image, subset, dst, sampling, nullptr,
                                  SkCanvas::kStrict_SrcRectConstraint);
            return {dst.width(), dst.height()};
        };

        auto draw_image_shader = [canvas](SkImage* image,
                                          const SkSamplingOptions& sampling) -> SkSize {
            if (!image) {
                return {0, 0};
            }
            SkMatrix m;
            m.setRotate(45, image->width()/2.f, image->height()/2.f);
            SkPaint paint;
            paint.setShader(image->makeShader(SkTileMode::kMirror, SkTileMode::kDecal,
                                              sampling, m));
            auto rect = SkRect::MakeWH(image->width() * 1.3f, image->height());
            canvas->drawRect(rect, paint);
            return {rect.width(), rect.height()};
        };

        canvas->translate(kPad, kPad);
        int imageIndex = 0;
        using DrawSig = SkSize(SkImage* image, const SkSamplingOptions&);
        using DF = std::function<DrawSig>;
        for (const auto& draw : {DF(draw_image), DF(draw_image_rect), DF(draw_image_shader)}) {
            float wForDrawFunc = 0;
            canvas->save();
            for (auto scale : {1.f, 1.5f, 0.3f}) {
                float hForScale = 0;
                float wForScale = 0;
                canvas->save();
                // We exercise either bicubic or mipmaps depending on the scale.
                SkSamplingOptions samplings[] = {
                        {SkFilterMode::kNearest},
                        {SkFilterMode::kLinear},
                        scale > 1.f
                                ? SkSamplingOptions{SkCubicResampler::CatmullRom()}
                                : SkSamplingOptions{SkFilterMode::kLinear, SkMipmapMode::kLinear}};

                for (const auto& sampling : samplings) {
                    float yuvAndRefH;
                    canvas->save();
                        canvas->scale(scale, scale);
                        auto s1 = draw(this->getYUVAImage(imageIndex++), sampling);
                        yuvAndRefH = kPad + std::ceil(scale * s1.height());
                    canvas->restore();
                    canvas->save();
                        canvas->translate(0, yuvAndRefH);
                        canvas->scale(scale, scale);
                        auto s2 = draw(fReferenceImage.get(), sampling);
                        yuvAndRefH += std::ceil(scale * s2.height());
                    canvas->restore();

                    float thisW = std::ceil(scale * std::max(s1.width(), s2.width()));

                    SkPaint outline;
                    outline.setColor(SK_ColorBLACK);
                    outline.setStroke(true);
                    outline.setAntiAlias(false);
                    canvas->drawRect(SkRect::MakeXYWH(-1, -1, thisW + 1, yuvAndRefH + 1), outline);

                    thisW += kPad;
                    yuvAndRefH += kPad;

                    canvas->translate(thisW, 0);

                    wForScale += thisW;
                    hForScale = std::max(hForScale, yuvAndRefH);
                }
                canvas->restore();
                canvas->translate(0, hForScale);
                wForDrawFunc = std::max(wForScale, wForDrawFunc);
            }
            canvas->restore();
            canvas->translate(wForDrawFunc, 0);
        }
     }

private:
    Source fSource;

    std::unique_ptr<sk_gpu_test::LazyYUVImage> fLazyYUVImage;

    // 3 draws x 3 scales x 4 filter qualities
    inline static constexpr int kNumImages = 3 * 3 * 4;
    sk_sp<SkImage> fYUVAImages[kNumImages];
    sk_sp<SkImage> fReferenceImage;

    inline static constexpr SkScalar kPad = 10.0f;

    using INHERITED = GM;
};

DEF_GM(return new ImageFromYUV(ImageFromYUV::Source::kTextures);)
DEF_GM(return new ImageFromYUV(ImageFromYUV::Source::kImages);)
}  // namespace skiagm
