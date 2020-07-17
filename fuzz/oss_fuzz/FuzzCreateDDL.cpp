/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "src/gpu/GrShaderCaps.h"
#include "tools/gpu/GrContextFactory.h"

#include "fuzz/Fuzz.h"

#include <iostream>

bool FuzzCreateDDL(Fuzz* fuzz) {
    //TODO (zepeng) create grcontext
    sk_sp<GrContext> context = GrContext::MakeGL();
    GrContext* gr = context.get();

    sk_gpu_test::GrContextFactory factory;
    sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(
        factory.ContextType::kANGLE_D3D11_ES2_ContextType
    );
    GrContext* context = ctxInfo.grContext();

    std::cout << context << std::endl;
    size_t maxResourceBytes = context->getResourceCacheLimit();

    SkSurfaceCharacterization c;
    if (!context->colorTypeSupportedAsSurface(kRGBA_8888_SkColorType)) {
        c = SkSurfaceCharacterization();
    } else {
        SkImageInfo ii = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType,
                                        kPremul_SkAlphaType, SkColorSpace::MakeSRGB());

        GrBackendFormat backendFormat = context->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                    GrRenderable::kYes);
        if (!backendFormat.isValid()) {
            c = SkSurfaceCharacterization();
        } else {
            c = context->threadSafeProxy()->createCharacterization(
                                        maxResourceBytes, ii, backendFormat, 1,
                                        kTopLeft_GrSurfaceOrigin, SkSurfaceProps(0x0, kRGB_H_SkPixelGeometry), true,
                                        false, true, GrProtected::kNo);
        }

    }
    if (!c.isValid()) {
        return false;
    }
    SkDeferredDisplayListRecorder r(c);
    SkCanvas* canvas = r.getCanvas();
    if (!canvas) {
        return false;
    }

    SkPaint paint;
    fuzz->next(&paint); // TODO(zepeng) should be same in FuzzCanvas.cpp
    SkRect tile;
    fuzz->next(&tile);

    canvas->drawRect(tile, paint);
    r.detach();
    return true;
}


#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 3000) {
        return 0;
    }
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzCreateDDL(bytes);
    return 0;
}
#endif
