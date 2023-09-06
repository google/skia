/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialSurface.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPixelRef.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSurfacePriv.h"

#include <memory>
#include <utility>

SkSpecialSurface::SkSpecialSurface(sk_sp<SkDevice> device, const SkIRect& subset)
        : fSubset(subset) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);

    fCanvas = std::make_unique<SkCanvas>(std::move(device));
    fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
    fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
}

sk_sp<SkSpecialImage> SkSpecialSurface::makeImageSnapshot() {
    fCanvas->restoreToCount(0);

    // Because of the above 'restoreToCount(0)' we know we're getting the root device here.
    SkDevice* rootDevice = SkCanvasPriv::TopDevice(fCanvas.get());
    if (!rootDevice) {
        return nullptr;
    }

    sk_sp<SkSpecialImage> image = rootDevice->snapSpecial(this->subset());

    fCanvas.reset();
    return image;
}

///////////////////////////////////////////////////////////////////////////////
namespace SkSpecialSurfaces {
sk_sp<SkSpecialSurface> MakeRaster(const SkImageInfo& info,
                                   const SkSurfaceProps& props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, 0);
    if (!pr) {
        return nullptr;
    }

    SkBitmap bitmap;
    bitmap.setInfo(info, info.minRowBytes());
    bitmap.setPixelRef(std::move(pr), 0, 0);

    sk_sp<SkDevice> device = sk_make_sp<SkBitmapDevice>(
            bitmap, props.cloneWithPixelGeometry(kUnknown_SkPixelGeometry));
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(info.dimensions());

    return sk_make_sp<SkSpecialSurface>(std::move(device), subset);
}
}  // namespace SkSpecialSurfaces
