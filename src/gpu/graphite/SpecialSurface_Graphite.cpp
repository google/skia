/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class SpecialSurface_Graphite : public SkSpecialSurface {
public:
    SpecialSurface_Graphite(sk_sp<Device> device, SkIRect subset)
            : SkSpecialSurface(subset, device->surfaceProps())
            , fView(device->readSurfaceView()) {
        SkASSERT(fView);

        fCanvas = std::make_unique<SkCanvas>(std::move(device));
        fCanvas->clipRect(SkRect::Make(subset));
    }

    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fView) {
            return nullptr;
        }

        // Note: SkSpecialImages can only be snapShotted once, so this call is destructive and we
        // move fView.
        return SkSpecialImage::MakeGraphite(fCanvas->recorder(),
                                            this->subset(),
                                            kNeedNewImageUniqueID_SpecialImage,
                                            std::move(fView),
                                            fCanvas->imageInfo().colorInfo(),
                                            this->props());
    }

private:
    TextureProxyView fView;
};

} // namespace skgpu::graphite

using namespace skgpu::graphite;

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeGraphite(Recorder* recorder,
                                                       const SkImageInfo& ii,
                                                       const SkSurfaceProps& props) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<Device> device = Device::Make(recorder, ii, SkBudgeted::kYes, props,
                                        /* addInitialClear= */ false);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SpecialSurface_Graphite>(std::move(device), subset);
}
