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
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "tools/gpu/GrContextFactory.h"

#include "fuzz/Fuzz.h"

#include <tuple>

/**
 * The fuzzer aims to fuzz the use of SkDeferredDisplayList. It mainly consists of
 * three parts.
 * 1. In create_surface_characterization, (make_characterization) Create SkSurfaceCharacterization
 * by using GrDirectContext of kGL_ContextType as it can be applied on all platform, and
 * (make_surface) create a GPU backend surface of the same GrDirectContext
 * 2. (make_ddl) Create SkDeferredDisplayListRecorder from the SkSurfaceCharacterization, and test
 * the recoder's corresponding canvas.
 * 3. (make_ddl, draw_ddl) Create SkDeferredDisplayList from the SkDeferredDisplayRecorder and draw
 * the ddl on a GPU backend surface.
 */

static constexpr int kMaxWidth = 64;
static constexpr int kMaxHeight = 64;
static constexpr int kSampleCount = 1;

static SkSurfaceProps gen_fuzzed_surface_props(Fuzz* fuzz) {
    SkPixelGeometry pixel;
    fuzz->nextEnum(&pixel, kBGR_V_SkPixelGeometry);
    return SkSurfaceProps(0x0, pixel);
}

static SkPaint gen_fuzzed_color(Fuzz* fuzz) {
    float R, G, B, Alpha;
    fuzz->nextRange(&R, -1, 2);
    fuzz->nextRange(&G, -1, 2);
    fuzz->nextRange(&B, -1, 2);
    fuzz->nextRange(&Alpha, 0, 1);
    SkColor4f color = {R, G, B, Alpha};
    return SkPaint(color);
}

static SkImageInfo gen_fuzzed_imageinfo(Fuzz* fuzz, SkColorType surfaceType) {
    SkAlphaType alphaType;
    fuzz->nextEnum(&alphaType, SkAlphaType::kLastEnum_SkAlphaType);
    int width, height;
    fuzz->nextRange(&width, 1, kMaxWidth);
    fuzz->nextRange(&height, 1, kMaxHeight);
    return SkImageInfo::Make(width, height, surfaceType, alphaType, SkColorSpace::MakeSRGB());
}

static SkSurfaceCharacterization make_characterization(Fuzz* fuzz, GrDirectContext* dContext,
                                                       SkImageInfo& ii, SkColorType surfaceType,
                                                       GrSurfaceOrigin origin) {
    if (!dContext->colorTypeSupportedAsSurface(surfaceType)) {
        SkDebugf("Color Type is not supported in the backend %s",
                 GrBackendApiToStr(dContext->backend()));
        return {};
    }

    GrBackendFormat backendFormat = dContext->defaultBackendFormat(surfaceType,
                                                                   GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        SkDebugf("Color Type is not supported in the backend %s",
                 GrBackendApiToStr(dContext->backend()));
        return {};
    }

    SkSurfaceCharacterization c;
    size_t maxResourceBytes = dContext->getResourceCacheLimit();
    c = dContext->threadSafeProxy()->createCharacterization(
                                maxResourceBytes, ii, backendFormat, kSampleCount,
                                origin, gen_fuzzed_surface_props(fuzz), true,
                                false, true, GrProtected::kNo);
#ifdef SK_VULKAN
    GrProtected protect;
    fuzz->nextEnum(&protect, GrProtected::kYes);
    c = dContext->threadSafeProxy()->createCharacterization(
                            maxResourceBytes, ii, backendFormat, kSampleCount,
                            origin, make_surface_prop(fuzz), true,
                            false, true, protect);
#endif

    if (!c.isValid()) {
        SkDebugf("Could not create Characterization in the backend %s",
                 GrBackendApiToStr(dContext->backend()));
        return {};
    }
    return c;
}

static sk_sp<SkDeferredDisplayList> make_ddl(Fuzz* fuzz, GrDirectContext* dContext,
                                             const SkSurfaceCharacterization& c) {
    SkDeferredDisplayListRecorder r(c);
    SkCanvas* canvas = r.getCanvas();
    if (!canvas) {
        SkDebugf("Could not create canvas for backend %s", GrBackendApiToStr(dContext->backend()));
        return nullptr;
    }
    // For now we only draw a rect into the DDL. This will be scaled up to draw more varied content.
    SkRect tile;
    fuzz->next(&tile);
    canvas->drawRect(tile, gen_fuzzed_color(fuzz));
    return r.detach();
}

static sk_sp<SkSurface> make_surface(Fuzz* fuzz, GrDirectContext* dContext, const SkImageInfo& ii,
                                     GrSurfaceOrigin origin) {
    SkBudgeted budget;
    fuzz->nextEnum(&budget, SkBudgeted::kYes);
    SkSurfaceProps surfaceProps = gen_fuzzed_surface_props(fuzz);
    auto surface = SkSurface::MakeRenderTarget(dContext, budget, ii, kSampleCount, origin,
                                               &surfaceProps);
    return surface;
}

static bool draw_ddl(sk_sp<SkSurface> surface, sk_sp<SkDeferredDisplayList> ddl) {
    surface->draw(std::move(ddl));
    if (!surface) {
        return false;
    }
    return true;
}

using SurfaceAndChar = std::tuple<sk_sp<SkSurface>, SkSurfaceCharacterization>;
static SurfaceAndChar create_surface_and_characterization(Fuzz* fuzz, GrDirectContext* dContext,
                                                          SkColorType surfaceType,
                                                          GrSurfaceOrigin origin) {
    SkImageInfo ii = gen_fuzzed_imageinfo(fuzz, surfaceType);
    SkSurfaceCharacterization c = make_characterization(fuzz, dContext, ii, surfaceType, origin);
    if (!c.isValid()) {
       return {};
    }

    auto surface = make_surface(fuzz, dContext, ii, origin);
    if (!surface) {
        return {};
    }
    return {surface, c};
}

DEF_FUZZ(CreateDDL, fuzz) {
    SkColorType surfaceType;
    GrSurfaceOrigin origin;
    fuzz->nextEnum(&surfaceType, SkColorType::kLastEnum_SkColorType);
    fuzz->nextEnum(&origin, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin);
    sk_gpu_test::GrContextFactory factory;
    sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(
        factory.ContextType::kGL_ContextType);
    GrDirectContext* dContext = ctxInfo.directContext();

    auto[surface, c] = create_surface_and_characterization(fuzz, dContext, surfaceType, origin);
    if (!surface) {
        return;
    }

    sk_sp<SkDeferredDisplayList> ddl = make_ddl(fuzz, dContext, c);
    if (!ddl) {
        SkDebugf("Could not create ddl %s", GrBackendApiToStr(dContext->backend()));
        return;
    }
    if (!draw_ddl(std::move(surface), std::move(ddl))) {
        SkDebugf("Could not draw ddl in the backend");
    }
    return;
}
