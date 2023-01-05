/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/shaders/SkImageShader.h"

namespace skgpu::graphite {

class SkSpecialImage_Graphite final : public SkSpecialImage {
public:
    SkSpecialImage_Graphite(Recorder* recorder,
                            const SkIRect& subset,
                            uint32_t uniqueID,
                            TextureProxyView view,
                            const SkColorInfo& colorInfo,
                            const SkSurfaceProps& props)
            : SkSpecialImage(subset, uniqueID, colorInfo, props)
            , fRecorder(recorder)
            , fTextureProxyView(std::move(view)) {
    }

    size_t getSize() const override {
        // TODO: return VRAM size here
        return 0;
    }

    void onDraw(SkCanvas* canvas,
                SkScalar x, SkScalar y,
                const SkSamplingOptions& sampling,
                const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        sk_sp<SkImage> img = sk_sp<SkImage>(new skgpu::graphite::Image(this->uniqueID(),
                                                                       fTextureProxyView,
                                                                       this->colorInfo()));

        canvas->drawImageRect(img, SkRect::Make(this->subset()), dst,
                              sampling, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

#if SK_SUPPORT_GPU
    GrSurfaceProxyView onView(GrRecordingContext*) const override {
        // To get here we would have to be requesting a Ganesh resource from a Graphite-backed
        // special image. That should never happen.
        SkASSERT(false);
        return {};
    }
#endif

    TextureProxyView onTextureProxyView() const override { return fTextureProxyView; }

    bool onGetROPixels(SkBitmap* dst) const override {
        // This should never be called: All GPU image filters are implemented entirely on the GPU,
        // so we never perform read-back.
        // TODO: re-enabled this assert once Graphite has image filter support. Right now image
        // filters will fallback to the raster backend in Graphite.
        //SkASSERT(false);
        return false;
    }

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType,
                                          const SkColorSpace* colorSpace,
                                          const SkISize& size,
                                          SkAlphaType at,
                                          const SkSurfaceProps& props) const override {
        SkASSERT(fRecorder);

        SkImageInfo ii = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));

        return SkSpecialSurface::MakeGraphite(fRecorder, ii, props);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeGraphite(fRecorder,
                                            subset,
                                            this->uniqueID(),
                                            fTextureProxyView,
                                            this->colorInfo(),
                                            this->props());
    }

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        if (subset) {
            // TODO: fill this in
            return nullptr;
        }

        return sk_make_sp<Image>(this->uniqueID(), fTextureProxyView, this->colorInfo());
    }

    sk_sp<SkShader> onAsShader(SkTileMode tileMode,
                               const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const override {
        // The special image's logical (0,0) is at its subset's topLeft() so we need to account for
        // that in the local matrix used when sampling.
        SkMatrix subsetOrigin = SkMatrix::Translate(-this->subset().topLeft());
        subsetOrigin.postConcat(lm);
        // However, we don't need to modify the subset itself since that is defined with respect to
        // the base image, and the local matrix is applied before any tiling/clamping.
        const SkRect subset = SkRect::Make(this->subset());

        // asImage() w/o a subset makes no copy; create the SkImageShader directly to remember the
        // subset used to access the image.
        return SkImageShader::MakeSubset(this->asImage(), subset, tileMode, tileMode,
                                         sampling, &subsetOrigin);
    }

    sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType,
                                        const SkColorSpace* colorSpace,
                                        const SkISize& size,
                                        SkAlphaType at) const override {
        // TODO (michaelludwig): Why does this ignore colorType but onMakeSurface doesn't ignore it?
        //    Once makeTightSurface() goes away, should this type overriding behavior be moved into
        //    onMakeSurface() or is this unnecessary?
        colorType = colorSpace && colorSpace->gammaIsLinear() ? kRGBA_F16_SkColorType
                                                              : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        // The user never gets a direct ref to this surface (nor its snapped image) so it must be
        // budgeted
        return Surface::MakeGraphite(fRecorder, info, skgpu::Budgeted::kYes);
    }

private:
    Recorder*        fRecorder;
    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

sk_sp<SkSpecialImage> SkSpecialImage::MakeGraphite(skgpu::graphite::Recorder* recorder,
                                                   const SkIRect& subset,
                                                   uint32_t uniqueID,
                                                   skgpu::graphite::TextureProxyView view,
                                                   const SkColorInfo& colorInfo,
                                                   const SkSurfaceProps& props) {
    if (!recorder || !view) {
        return nullptr;
    }

    SkASSERT(RectFits(subset, view.width(), view.height()));
    return sk_make_sp<skgpu::graphite::SkSpecialImage_Graphite>(recorder, subset, uniqueID,
                                                                std::move(view), colorInfo, props);
}
