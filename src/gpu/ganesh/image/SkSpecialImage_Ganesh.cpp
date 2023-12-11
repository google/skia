/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"

#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/shaders/SkImageShader.h"

#include <cstddef>
#include <utility>

class SkShader;
struct SkSamplingOptions;
enum SkColorType : int;
enum class SkTileMode;

class SkSpecialImage_Gpu final : public SkSpecialImage {
public:
    SkSpecialImage_Gpu(GrRecordingContext* context,
                       const SkIRect& subset,
                       uint32_t uniqueID,
                       GrSurfaceProxyView view,
                       const SkColorInfo& colorInfo,
                       const SkSurfaceProps& props)
            : SkSpecialImage(subset, uniqueID, colorInfo, props)
            , fContext(context)
            , fView(std::move(view)) {}

    size_t getSize() const override { return fView.proxy()->gpuMemorySize(); }

    bool isGaneshBacked() const override { return true; }

    GrRecordingContext* getContext() const override { return fContext; }

    GrSurfaceProxyView view(GrRecordingContext*) const { return fView; }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImages::MakeDeferredFromGpu(
                fContext, subset, this->uniqueID(), fView, this->colorInfo(), this->props());
    }

    sk_sp<SkImage> asImage() const override {
        fView.proxy()->priv().exactify(true);
        return sk_make_sp<SkImage_Ganesh>(
                sk_ref_sp(fContext), this->uniqueID(), fView, this->colorInfo());
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
        return SkImageShader::MakeSubset(
                this->asImage(), subset, tileMode, tileMode, sampling, &subsetOrigin);
    }

private:
    GrRecordingContext* fContext;
    GrSurfaceProxyView fView;
};

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromTextureImage(GrRecordingContext* rContext,
                                           const SkIRect& subset,
                                           sk_sp<SkImage> image,
                                           const SkSurfaceProps& props) {
    if (!rContext || !image || subset.isEmpty()) {
        return nullptr;
    }

    SkASSERT(image->bounds().contains(subset));

    // This will work even if the image is a raster-backed image.
    auto [view, ct] = skgpu::ganesh::AsView(rContext, image, skgpu::Mipmapped::kNo);
    return MakeDeferredFromGpu(rContext,
                               subset,
                               image->uniqueID(),
                               std::move(view),
                               {ct, image->alphaType(), image->refColorSpace()},
                               props);
}

sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext* context,
                                          const SkIRect& subset,
                                          uint32_t uniqueID,
                                          GrSurfaceProxyView view,
                                          const GrColorInfo& colorInfo,
                                          const SkSurfaceProps& props) {
    if (!context || context->abandoned() || !view.asTextureProxy()) {
        return nullptr;
    }

    SkASSERT(view.proxy()->backingStoreBoundsIRect().contains(subset));

    SkColorType ct = GrColorTypeToSkColorType(colorInfo.colorType());
    return sk_make_sp<SkSpecialImage_Gpu>(
            context,
            subset,
            uniqueID,
            std::move(view),
            SkColorInfo(ct, colorInfo.alphaType(), colorInfo.refColorSpace()),
            props);
}

GrSurfaceProxyView AsView(GrRecordingContext* context, const SkSpecialImage* img) {
    if (!context || !img || !img->isGaneshBacked()) {
        return {};
    }
    auto grImg = static_cast<const SkSpecialImage_Gpu*>(img);
    return grImg->view(context);
}

}  // namespace SkSpecialImages
