/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendObject.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"


static sk_sp<GrBackendObject> create_by_colortype(GrContext* context, GrRenderable renderable) {
    return GrBackendObject::Make(context, 32, 32, kRGBA_8888_SkColorType,
                                 GrMipMapped::kNo, renderable);
}

static sk_sp<GrBackendObject> create_by_format(GrContext* context, GrRenderable renderable) {
    GrBackendFormat format =
                context->priv().caps()->getBackendFormatFromColorType(kRGBA_8888_SkColorType);

    return GrBackendObject::Make(context, 32, 32, format, GrMipMapped::kNo, renderable);
}

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   sk_sp<GrBackendObject> (*createMtd)(GrContext*, GrRenderable),
                   GrRenderable renderable) {
    GrResourceCache* cache = context->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    sk_sp<GrBackendObject> o = createMtd(context, renderable);;
    REPORTER_ASSERT(reporter, o);
    if (!o) {
        return;
    }
    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    if (GrRenderable::kYes == renderable) {
        sk_sp<SkSurface> s = SkSurface::MakeFromBackendTexture(context,
                                                               o->backendTexture(),
                                                               kTopLeft_GrSurfaceOrigin,
                                                               0,
                                                               kRGBA_8888_SkColorType,
                                                               nullptr, nullptr);
        REPORTER_ASSERT(reporter, s);
        REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
    }

    {
        sk_sp<SkImage> i = SkImage::MakeFromTexture(context,
                                                    o->backendTexture(),
                                                    kTopLeft_GrSurfaceOrigin,
                                                    kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType,
                                                    nullptr);
        REPORTER_ASSERT(reporter, i);
        REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());
    o.reset();
}



DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    test_wrapping(context, reporter, create_by_colortype, GrRenderable::kNo);
    test_wrapping(context, reporter, create_by_colortype, GrRenderable::kYes);
    test_wrapping(context, reporter, create_by_format, GrRenderable::kNo);
    test_wrapping(context, reporter, create_by_format, GrRenderable::kYes);
}
