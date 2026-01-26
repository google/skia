/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/ganesh/GrCanvas.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/image/GrMippedBitmap.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

DEF_SIMPLE_GPU_GM(swizzle, rContext, canvas, 512, 512) {
    auto sfc = skgpu::ganesh::TopDeviceSurfaceFillContext(canvas);
    if (!sfc) {
        return;
    }

    SkBitmap bmp;
    ToolUtils::GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    auto bitmap = GrMippedBitmap::Make(bmp.pixmap());
    SkASSERT_RELEASE(bitmap);
    SkAlphaType alphaType = bitmap->alphaType();
    auto view = std::get<0>(GrMakeCachedBitmapProxyView(
            rContext, bitmap.value(), /*label=*/"Gm_Swizzle", skgpu::Mipmapped::kNo));
    if (!view) {
        return;
    }
    std::unique_ptr<GrFragmentProcessor> imgFP =
            GrTextureEffect::Make(std::move(view), alphaType, SkMatrix());
    auto fp = GrFragmentProcessor::SwizzleOutput(std::move(imgFP), skgpu::Swizzle("grb1"));

    sfc->fillWithFP(std::move(fp));
}
