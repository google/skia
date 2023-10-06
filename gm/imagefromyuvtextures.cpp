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
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkYUVMath.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/gpu/YUVUtils.h"

namespace skiagm {
class ImageFromYUVTextures : public GM {
public:
    ImageFromYUVTextures() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override { return SkString("image_from_yuv_textures"); }

    SkISize getISize() override { return {1420, 610}; }

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
        return sk_gpu_test::LazyYUVImage::Make(std::move(pixmaps));
    }

    sk_sp<SkImage> makeYUVAImage(GrDirectContext* context) {
        return fLazyYUVImage->refImage(context, sk_gpu_test::LazyYUVImage::Type::kFromTextures);
    }

    sk_sp<SkImage> createReferenceImage(GrDirectContext* dContext) {
        auto planarImage = this->makeYUVAImage(dContext);
        if (!planarImage) {
            return nullptr;
        }

        auto resultInfo = SkImageInfo::Make(fLazyYUVImage->dimensions(),
                                            kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType);
        auto resultSurface = SkSurfaces::RenderTarget(
                dContext, skgpu::Budgeted::kYes, resultInfo, 1, kTopLeft_GrSurfaceOrigin, nullptr);
        if (!resultSurface) {
            return nullptr;
        }

        resultSurface->getCanvas()->drawImage(std::move(planarImage), 0, 0);
        return resultSurface->makeImageSnapshot();
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext || dContext->abandoned()) {
            *errorMsg = "DirectContext required to create YUV images";
            return DrawResult::kSkip;
        }

        if (!fLazyYUVImage) {
            fLazyYUVImage = CreatePlanes("images/mandrill_32.png");
        }

        // We make a version of this image for each draw because, if any draw flattens it to
        // RGBA, then all subsequent draws would use the RGBA texture.
        for (int i = 0; i < kNumImages; ++i) {
            fYUVAImages[i] = this->makeYUVAImage(dContext);
            if (!fYUVAImages[i]) {
                *errorMsg = "Couldn't create src YUVA image.";
                return DrawResult::kFail;
            }
        }

        fReferenceImage = this->createReferenceImage(dContext);
        if (!fReferenceImage) {
            *errorMsg = "Couldn't create reference YUVA image.";
            return DrawResult::kFail;
        }

        // Some backends (e.g., Vulkan) require all work be completed for backend textures
        // before they are deleted. Since we don't know when we'll next have access to a
        // direct context, flush all the work now.
        dContext->flush();
        dContext->submit(GrSyncCpu::kYes);

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
            for (auto scale : {1.f, 4.f, 0.75f}) {
                SkScalar h = 0;
                canvas->save();
                for (const auto& sampling : {
                    SkSamplingOptions(SkFilterMode::kNearest),
                    SkSamplingOptions(SkFilterMode::kLinear),
                    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest),
                    SkSamplingOptions(SkCubicResampler::Mitchell())})
                {
                    canvas->save();
                        canvas->scale(scale, scale);
                        auto s1 = draw(this->getYUVAImage(imageIndex++), sampling);
                    canvas->restore();
                    canvas->translate(kPad + SkScalarCeilToScalar(scale*s1.width()), 0);
                    canvas->save();
                        canvas->scale(scale, scale);
                        auto s2 = draw(fReferenceImage.get(), sampling);
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
    std::unique_ptr<sk_gpu_test::LazyYUVImage> fLazyYUVImage;

    // 3 draws x 3 scales x 4 filter qualities
    inline static constexpr int kNumImages = 3 * 3 * 4;
    sk_sp<SkImage> fYUVAImages[kNumImages];
    sk_sp<SkImage> fReferenceImage;

    inline static constexpr SkScalar kPad = 10.0f;

    using INHERITED = GM;
};

DEF_GM(return new ImageFromYUVTextures;)
}  // namespace skiagm
