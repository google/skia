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
 * The fuzzer aim to fuzz the creation of SkDeferredDisplayList. It mainly consist of
 * three parts.
 * 1. In create_surface_characterization, (make_characterization) Create SkSurfaceCharacterization
 * by using GrDirectContext of kGL_ContextType as it can be applied on all platform, and
 * (make_surface) create a GPU backend surface of the same GrDirectContext
 * 2. (make_ddl) Create SkDeferredDisplayListRecorder from the SkSurfaceCharacterization, and test
 * the recoder's corresponding canvas.
 * 3. (make_ddl, draw_ddl) Create SkDeferredDisplayList from the SkDeferredDisplayRecorder and draw
 * the ddl on a GPU backend surface.
 */

static constexpr int kWidth = 64;
static constexpr int kHeight = 64;

static SkSurfaceCharacterization make_characterization(Fuzz* fuzz, GrDirectContext* dContext, SkImageInfo ii) {
    if (!dContext->colorTypeSupportedAsSurface(kRGBA_8888_SkColorType)) {
        SkDebugf("kRGBA_8888 is not supported in the backend %s",
            GrBackendApiToStr(dContext->backend()));
        return {};
    }

    GrBackendFormat backendFormat = dContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        SkDebugf("kRGBA_8888 is not supported in the backend %s",
            GrBackendApiToStr(dContext->backend()));
        return {};
    }

    SkSurfaceCharacterization c;
    size_t maxResourceBytes = dContext->getResourceCacheLimit();
    SkPixelGeometry pixel;
    fuzz->nextEnum(&pixel, kBGR_V_SkPixelGeometry);
    c = dContext->threadSafeProxy()->createCharacterization(
                                maxResourceBytes, ii, backendFormat, 1,
                                kTopLeft_GrSurfaceOrigin, SkSurfaceProps(0x0, pixel), true,
                                false, true, GrProtected::kNo);


    if (!c.isValid()) {
        SkDebugf("Could not create Characterization %s",
            GrBackendApiToStr(dContext->backend()));
        return {};
    }
    return c;
}
static sk_sp<SkDeferredDisplayList> make_ddl(Fuzz* fuzz, GrDirectContext* dContext, SkSurfaceCharacterization c) {
    SkDeferredDisplayListRecorder r(c);
    SkCanvas* canvas = r.getCanvas();
    if (!canvas) {
        SkDebugf("Could not create canvas %s",
            GrBackendApiToStr(dContext->backend()));
        return nullptr;
    }

    SkRect tile;
    fuzz->next(&tile);
    SkColor4f color;
    float R, G, B, Alpha;
    fuzz->nextRange(&R, -1, 2);
    fuzz->nextRange(&G, -1, 2);
    fuzz->nextRange(&B, -1, 2);
    fuzz->nextRange(&Alpha, 0, 1);
    color = {R, G, B, Alpha};
    SkPaint paint = SkPaint(color);
    canvas->drawRect(tile, SkPaint());
    return r.detach();
}

static sk_sp<SkSurface> make_surface(Fuzz* fuzz, GrDirectContext* dContext, const SkImageInfo& ii) {
    SkBudgeted budget;
    fuzz->nextEnum(&budget, SkBudgeted::kYes);
    GrSurfaceOrigin origin;
    fuzz->nextEnum(&origin, GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin);
    auto surface = SkSurface::MakeRenderTarget(dContext, budget, ii, 1, origin, nullptr);
    return surface;
}

static void draw_ddl(sk_sp<SkSurface> surface, sk_sp<SkDeferredDisplayList> ddl) {
    surface->draw(std::move(ddl));
}

static std::tuple<sk_sp<SkSurface>, SkSurfaceCharacterization> create_surface_characterization(Fuzz* fuzz, GrDirectContext* dContext) {

    SkColorType colorType;
    fuzz->nextEnum(&colorType, SkColorType::kLastEnum_SkColorType);
    SkAlphaType alphaType;
    fuzz->nextEnum(&alphaType, SkAlphaType::kLastEnum_SkAlphaType);
    SkImageInfo ii = SkImageInfo::Make(kWidth, kHeight, colorType,
                                    alphaType, SkColorSpace::MakeSRGB());
    SkSurfaceCharacterization c = make_characterization(fuzz, dContext, ii);
    if (!c.isValid()) {
       return {};
    }

    auto surface = make_surface(fuzz, dContext, ii);
    if (!surface) {
        return {};
    }
    return {surface, c};
}

DEF_FUZZ(CreateDDL, fuzz) {
    sk_gpu_test::GrContextFactory factory;
    sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(
        factory.ContextType::kGL_ContextType);
    GrDirectContext* dContext = ctxInfo.directContext();

    std::tuple<sk_sp<SkSurface>, SkSurfaceCharacterization> tuple =
        create_surface_characterization(fuzz, dContext);
    if (!std::tuple_size<decltype(tuple)>::value) {
        return;
    }
    auto surface = std::get<0>(tuple);
    auto c = std::get<1>(tuple);

    sk_sp<SkDeferredDisplayList> ddl = make_ddl(fuzz, dContext, c);
    if (!ddl) {
        SkDebugf("Could not create ddl %s",
            GrBackendApiToStr(dContext->backend()));
        return;
    }
    draw_ddl(std::move(surface), std::move(ddl));
    return;
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
