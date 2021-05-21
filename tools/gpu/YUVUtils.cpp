/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/YUVUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/core/SkYUVMath.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "tools/gpu/ManagedBackendTexture.h"

namespace {

static SkPMColor convert_yuva_to_rgba(const float mtx[20], uint8_t yuva[4]) {
    uint8_t y = yuva[0];
    uint8_t u = yuva[1];
    uint8_t v = yuva[2];
    uint8_t a = yuva[3];

    uint8_t r = SkTPin(SkScalarRoundToInt(mtx[ 0]*y + mtx[ 1]*u + mtx[ 2]*v + mtx[ 4]*255), 0, 255);
    uint8_t g = SkTPin(SkScalarRoundToInt(mtx[ 5]*y + mtx[ 6]*u + mtx[ 7]*v + mtx[ 9]*255), 0, 255);
    uint8_t b = SkTPin(SkScalarRoundToInt(mtx[10]*y + mtx[11]*u + mtx[12]*v + mtx[14]*255), 0, 255);

    return SkPremultiplyARGBInline(a, r, g, b);
}

static uint8_t look_up(SkPoint normPt, const SkPixmap& pmap, SkColorChannel channel) {
    SkASSERT(normPt.x() > 0 && normPt.x() < 1.0f);
    SkASSERT(normPt.y() > 0 && normPt.y() < 1.0f);
    int x = SkScalarFloorToInt(normPt.x() * pmap.width());
    int y = SkScalarFloorToInt(normPt.y() * pmap.height());

    auto ii = pmap.info().makeColorType(kRGBA_8888_SkColorType).makeWH(1, 1);
    uint32_t pixel;
    SkAssertResult(pmap.readPixels(ii, &pixel, sizeof(pixel), x, y));
    int shift = static_cast<int>(channel) * 8;
    return static_cast<uint8_t>((pixel >> shift) & 0xff);
}

class Generator : public SkImageGenerator {
public:
    Generator(SkYUVAPixmaps pixmaps, sk_sp<SkColorSpace> cs)
            : SkImageGenerator(SkImageInfo::Make(pixmaps.yuvaInfo().dimensions(),
                                                 kN32_SkColorType,
                                                 kPremul_SkAlphaType,
                                                 std::move(cs)))
            , fPixmaps(std::move(pixmaps)) {}

protected:
    bool onGetPixels(const SkImageInfo& info,
                     void* pixels,
                     size_t rowBytes,
                     const Options&) override {
        if (kUnknown_SkColorType == fFlattened.colorType()) {
            fFlattened.allocPixels(info);
            SkASSERT(info == this->getInfo());

            float mtx[20];
            SkColorMatrix_YUV2RGB(fPixmaps.yuvaInfo().yuvColorSpace(), mtx);
            SkYUVAInfo::YUVALocations yuvaLocations = fPixmaps.toYUVALocations();
            SkASSERT(SkYUVAInfo::YUVALocation::AreValidLocations(yuvaLocations));

            SkMatrix om = fPixmaps.yuvaInfo().originMatrix();
            SkAssertResult(om.invert(&om));
            float normX = 1.f/info.width();
            float normY = 1.f/info.height();
            if (SkEncodedOriginSwapsWidthHeight(fPixmaps.yuvaInfo().origin())) {
                using std::swap;
                swap(normX, normY);
            }
            for (int y = 0; y < info.height(); ++y) {
                for (int x = 0; x < info.width(); ++x) {
                    SkPoint xy1 {(x + 0.5f),
                                 (y + 0.5f)};
                    xy1 = om.mapPoint(xy1);
                    xy1.fX *= normX;
                    xy1.fY *= normY;

                    uint8_t yuva[4] = {0, 0, 0, 255};

                    for (auto c : {SkYUVAInfo::YUVAChannels::kY,
                                   SkYUVAInfo::YUVAChannels::kU,
                                   SkYUVAInfo::YUVAChannels::kV}) {
                        const auto& pmap = fPixmaps.plane(yuvaLocations[c].fPlane);
                        yuva[c] = look_up(xy1, pmap, yuvaLocations[c].fChannel);
                    }
                    auto [aPlane, aChan] = yuvaLocations[SkYUVAInfo::YUVAChannels::kA];
                    if (aPlane >= 0) {
                        const auto& pmap = fPixmaps.plane(aPlane);
                        yuva[3] = look_up(xy1, pmap, aChan);
                    }

                    // Making premul here.
                    *fFlattened.getAddr32(x, y) = convert_yuva_to_rgba(mtx, yuva);
                }
            }
        }

        return fFlattened.readPixels(info, pixels, rowBytes, 0, 0);
    }

    bool onQueryYUVAInfo(const SkYUVAPixmapInfo::SupportedDataTypes& types,
                         SkYUVAPixmapInfo* info) const override {
        *info = fPixmaps.pixmapsInfo();
        return info->isValid();
    }

    bool onGetYUVAPlanes(const SkYUVAPixmaps& pixmaps) override {
        SkASSERT(pixmaps.yuvaInfo() == fPixmaps.yuvaInfo());
        for (int i = 0; i < pixmaps.numPlanes(); ++i) {
            SkASSERT(fPixmaps.plane(i).colorType() == pixmaps.plane(i).colorType());
            SkASSERT(fPixmaps.plane(i).dimensions() == pixmaps.plane(i).dimensions());
            SkASSERT(fPixmaps.plane(i).rowBytes() == pixmaps.plane(i).rowBytes());
            fPixmaps.plane(i).readPixels(pixmaps.plane(i));
        }
        return true;
    }

private:
    SkYUVAPixmaps fPixmaps;
    SkBitmap      fFlattened;
};

}  // anonymous namespace

namespace sk_gpu_test {

std::tuple<std::array<sk_sp<SkImage>, SkYUVAInfo::kMaxPlanes>, SkYUVAInfo>
MakeYUVAPlanesAsA8(SkImage* src,
                   SkYUVColorSpace cs,
                   SkYUVAInfo::Subsampling ss,
                   GrRecordingContext* rContext) {
    float rgbToYUV[20];
    SkColorMatrix_RGB2YUV(cs, rgbToYUV);

    SkYUVAInfo::PlaneConfig config = src->isOpaque() ? SkYUVAInfo::PlaneConfig::kY_U_V
                                                     : SkYUVAInfo::PlaneConfig::kY_U_V_A;
    SkISize dims[SkYUVAInfo::kMaxPlanes];
    int n = SkYUVAInfo::PlaneDimensions(src->dimensions(),
                                        config,
                                        ss,
                                        kTopLeft_SkEncodedOrigin,
                                        dims);
    std::array<sk_sp<SkImage>, 4> planes;
    for (int i = 0; i < n; ++i) {
        SkImageInfo info = SkImageInfo::MakeA8(dims[i]);
        sk_sp<SkSurface> surf;
        if (rContext) {
            surf = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, info, 1, nullptr);
        } else {
            surf = SkSurface::MakeRaster(info);
        }
        if (!surf) {
            return {};
        }

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);

        // Make a matrix with the ith row of rgbToYUV copied to the A row since we're drawing to A8.
        float m[20] = {};
        std::copy_n(rgbToYUV + 5*i, 5, m + 15);
        paint.setColorFilter(SkColorFilters::Matrix(m));
        surf->getCanvas()->drawImageRect(src,
                                         SkRect::Make(dims[i]),
                                         SkSamplingOptions(SkFilterMode::kLinear),
                                         &paint);
        planes[i] = surf->makeImageSnapshot();
        if (!planes[i]) {
            return {};
        }
    }
    SkYUVAInfo info(src->dimensions(), config, ss, cs);
    return {planes, info};
}

std::unique_ptr<LazyYUVImage> LazyYUVImage::Make(sk_sp<SkData> data,
                                                 GrMipmapped mipmapped,
                                                 sk_sp<SkColorSpace> cs) {
    std::unique_ptr<LazyYUVImage> image(new LazyYUVImage());
    if (image->reset(std::move(data), mipmapped, std::move(cs))) {
        return image;
    } else {
        return nullptr;
    }
}

std::unique_ptr<LazyYUVImage> LazyYUVImage::Make(SkYUVAPixmaps pixmaps,
                                                 GrMipmapped mipmapped,
                                                 sk_sp<SkColorSpace> cs) {
    std::unique_ptr<LazyYUVImage> image(new LazyYUVImage());
    if (image->reset(std::move(pixmaps), mipmapped, std::move(cs))) {
        return image;
    } else {
        return nullptr;
    }
}

sk_sp<SkImage> LazyYUVImage::refImage(GrRecordingContext* rContext, Type type) {
    if (this->ensureYUVImage(rContext, type)) {
        size_t idx = static_cast<size_t>(type);
        SkASSERT(idx < SK_ARRAY_COUNT(fYUVImage));
        return fYUVImage[idx];
    } else {
        return nullptr;
    }
}

bool LazyYUVImage::reset(sk_sp<SkData> data, GrMipmapped mipmapped, sk_sp<SkColorSpace> cs) {
    fMipmapped = mipmapped;
    auto codec = SkCodecImageGenerator::MakeFromEncodedCodec(data);
    if (!codec) {
        return false;
    }

    SkYUVAPixmapInfo yuvaPixmapInfo;
    if (!codec->queryYUVAInfo(SkYUVAPixmapInfo::SupportedDataTypes::All(), &yuvaPixmapInfo)) {
        return false;
    }
    fPixmaps = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    if (!fPixmaps.isValid()) {
        return false;
    }

    if (!codec->getYUVAPlanes(fPixmaps)) {
        return false;
    }

    fColorSpace = std::move(cs);

    // The SkPixmap data is fully configured now for MakeFromYUVAPixmaps once we get a GrContext
    return true;
}

bool LazyYUVImage::reset(SkYUVAPixmaps pixmaps, GrMipmapped mipmapped, sk_sp<SkColorSpace> cs) {
    if (!pixmaps.isValid()) {
        return false;
    }
    fMipmapped = mipmapped;
    if (pixmaps.ownsStorage()) {
        fPixmaps = std::move(pixmaps);
    } else {
        fPixmaps = SkYUVAPixmaps::MakeCopy(std::move(pixmaps));
    }
    fColorSpace = std::move(cs);
    // The SkPixmap data is fully configured now for MakeFromYUVAPixmaps once we get a GrContext
    return true;
}

bool LazyYUVImage::ensureYUVImage(GrRecordingContext* rContext, Type type) {
    size_t idx = static_cast<size_t>(type);
    SkASSERT(idx < SK_ARRAY_COUNT(fYUVImage));
    if (fYUVImage[idx] && fYUVImage[idx]->isValid(rContext)) {
        return true;  // Have already made a YUV image valid for this context.
    }
    // Try to make a new YUV image for this context.
    switch (type) {
        case Type::kFromPixmaps:
            if (!rContext || rContext->abandoned()) {
                return false;
            }
            fYUVImage[idx] = SkImage::MakeFromYUVAPixmaps(rContext,
                                                          fPixmaps,
                                                          fMipmapped,
                                                          /*limit to max tex size*/ false,
                                                          fColorSpace);
            break;
        case Type::kFromGenerator: {
            // Make sure the generator has ownership of its backing planes.
            auto generator = std::make_unique<Generator>(fPixmaps, fColorSpace);
            fYUVImage[idx] = SkImage::MakeFromGenerator(std::move(generator));
            break;
        }
        case Type::kFromTextures:
            if (!rContext || rContext->abandoned()) {
                return false;
            }
            if (fMipmapped == GrMipmapped::kYes) {
                // If this becomes necessary we should invoke SkMipmapBuilder here to make mip
                // maps from our src data (and then pass a pixmaps array to initialize the planar
                // textures.
                return false;
            }
            if (auto direct = rContext->asDirectContext()) {
                sk_sp<sk_gpu_test::ManagedBackendTexture> mbets[SkYUVAInfo::kMaxPlanes];
                GrBackendTexture textures[SkYUVAInfo::kMaxPlanes];
                for (int i = 0; i < fPixmaps.numPlanes(); ++i) {
                    mbets[i] = sk_gpu_test::ManagedBackendTexture::MakeWithData(
                            direct,
                            fPixmaps.plane(i),
                            kTopLeft_GrSurfaceOrigin,
                            GrRenderable::kNo,
                            GrProtected::kNo);
                    if (mbets[i]) {
                        textures[i] = mbets[i]->texture();
                    } else {
                        return false;
                    }
                }
                GrYUVABackendTextures yuvaTextures(fPixmaps.yuvaInfo(),
                                                   textures,
                                                   kTopLeft_GrSurfaceOrigin);
                if (!yuvaTextures.isValid()) {
                    return false;
                }
                void* planeRelContext =
                        sk_gpu_test::ManagedBackendTexture::MakeYUVAReleaseContext(mbets);
                fYUVImage[idx] = SkImage::MakeFromYUVATextures(
                        direct,
                        yuvaTextures,
                        fColorSpace,
                        sk_gpu_test::ManagedBackendTexture::ReleaseProc,
                        planeRelContext);
            }
    }
    return fYUVImage[idx] != nullptr;
}
} // namespace sk_gpu_test
