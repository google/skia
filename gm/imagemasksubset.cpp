/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Surface.h"
#endif

namespace {

const SkISize   kSize = SkISize::Make(100, 100);
const SkIRect kSubset = SkIRect::MakeLTRB(25, 25, 75, 75);
const SkRect    kDest = SkRect::MakeXYWH(10, 10, 100, 100);

sk_sp<SkImage> make_mask(const sk_sp<SkSurface>& surface) {
    ToolUtils::draw_checkerboard(surface->getCanvas(), 0x80808080, 0x00000000, 5);
    return surface->makeImageSnapshot();
}

class MaskGenerator final : public SkImageGenerator {
public:
    MaskGenerator(const SkImageInfo& info) : INHERITED(info) {}

    bool onGetPixels(const SkImageInfo& info, void* pixels,
                     size_t rowBytes, const Options&) override {
        SkImageInfo surfaceInfo = info;
        if (kAlpha_8_SkColorType == info.colorType()) {
            surfaceInfo = surfaceInfo.makeColorSpace(nullptr);
        }

        make_mask(SkSurfaces::WrapPixels(surfaceInfo, pixels, rowBytes));
        return true;
    }

private:
    using INHERITED = SkImageGenerator;
};

using MakerT = sk_sp<SkImage>(*)(SkCanvas*, const SkImageInfo&);
const MakerT makers[] = {
        // SkImage_Raster
        [](SkCanvas*, const SkImageInfo& info) -> sk_sp<SkImage> {
            return make_mask(SkSurfaces::Raster(info));
        },

        // SkImage_Ganesh
        [](SkCanvas* c, const SkImageInfo& info) -> sk_sp<SkImage> {
            sk_sp<SkSurface> surface;
            if (c->recordingContext()) {
                surface =
                        SkSurfaces::RenderTarget(c->recordingContext(), skgpu::Budgeted::kNo, info);
            } else {
#if defined(SK_GRAPHITE)
                surface = SkSurfaces::RenderTarget(c->recorder(), info);
#endif
            }
            return make_mask(surface ? surface : SkSurfaces::Raster(info));
        },

        // SkImage_Lazy
        [](SkCanvas*, const SkImageInfo& info) -> sk_sp<SkImage> {
            return SkImages::DeferredFromGenerator(std::make_unique<MaskGenerator>(info));
        },
};

}  // namespace

// Checks whether subset SkImages preserve the original color type (A8 in this case).
DEF_SIMPLE_GM(imagemasksubset, canvas, 480, 480) {
    SkPaint paint;
    paint.setColor(0xff00ff00);

    const SkImageInfo info = SkImageInfo::MakeA8(kSize.width(), kSize.height());

    for (size_t i = 0; i < std::size(makers); ++i) {
        sk_sp<SkImage> image = ToolUtils::MakeTextureImage(canvas, makers[i](canvas, info));
        if (image) {
            canvas->drawImageRect(image, SkRect::Make(kSubset), kDest, SkSamplingOptions(),
                                  &paint, SkCanvas::kStrict_SrcRectConstraint);
            sk_sp<SkImage> subset;

            if (auto direct = GrAsDirectContext(canvas->recordingContext())) {
                subset = image->makeSubset(direct, kSubset);
            } else {
#if defined(SK_GRAPHITE)
                subset = image->makeSubset(canvas->recorder(), kSubset, {});
#endif
            }

            canvas->drawImageRect(subset, kDest.makeOffset(kSize.width() * 1.5f, 0),
                                  SkSamplingOptions(), &paint);
        }
        canvas->translate(0, kSize.height() * 1.5f);
    }
}
