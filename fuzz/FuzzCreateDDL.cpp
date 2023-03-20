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
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
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

static SkPaint gen_fuzzed_skpaint(Fuzz* fuzz) {
    float R, G, B, Alpha;
    fuzz->nextRange(&R, -1, 2);
    fuzz->nextRange(&G, -1, 2);
    fuzz->nextRange(&B, -1, 2);
    fuzz->nextRange(&Alpha, 0, 1);
    SkColor4f color = {R, G, B, Alpha};
    return SkPaint(color);
}

static SkImageInfo gen_fuzzed_imageinfo(Fuzz* fuzz, SkColorType surfaceType) {
    int width, height;
    fuzz->nextRange(&width, 1, kMaxWidth);
    fuzz->nextRange(&height, 1, kMaxHeight);
    SkAlphaType alphaType;
    fuzz->nextEnum(&alphaType, SkAlphaType::kLastEnum_SkAlphaType);
    skcms_TransferFunction skcmsFn;
    uint8_t skcms;
    fuzz->nextRange(&skcms, 0, 5);
    switch (skcms) {
        case 0: {
            skcmsFn = SkNamedTransferFn::kSRGB;
            break;
        }
        case 1: {
            skcmsFn = SkNamedTransferFn::k2Dot2;
            break;
        }
        case 2: {
            skcmsFn = SkNamedTransferFn::kHLG;
            break;
        }
        case 3: {
            skcmsFn = SkNamedTransferFn::kLinear;
            break;
        }
        case 4: {
            skcmsFn = SkNamedTransferFn::kPQ;
            break;
        }
        case 5: {
            skcmsFn = SkNamedTransferFn::kRec2020;
            break;
        }
        default:
            SkASSERT(false);
            break;
    }
    skcms_Matrix3x3 skcmsMat;
    fuzz->nextRange(&skcms, 0, 4);
    switch (skcms) {
        case 0: {
            skcmsMat = SkNamedGamut::kAdobeRGB;
            break;
        }
        case 1: {
            skcmsMat = SkNamedGamut::kDisplayP3;
            break;
        }
        case 2: {
            skcmsMat = SkNamedGamut::kRec2020;
            break;
        }
        case 3: {
            skcmsMat = SkNamedGamut::kSRGB;
            break;
        }
        case 4: {
            skcmsMat = SkNamedGamut::kXYZ;
            break;
        }
        default:
            SkASSERT(false);
            break;
    }
    return SkImageInfo::Make(width, height, surfaceType, alphaType,
                             SkColorSpace::MakeRGB(skcmsFn, skcmsMat));
}

static SkSurfaceCharacterization make_characterization(Fuzz* fuzz, GrDirectContext* dContext,
                                                       SkImageInfo& ii, SkColorType surfaceType,
                                                       GrSurfaceOrigin origin) {
    if (!dContext->colorTypeSupportedAsSurface(surfaceType)) {
        SkDebugf("Couldn't create backend texture in the backend %s",
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
    GrProtected protect = GrProtected::kNo;
#ifdef SK_VULKAN
    fuzz->nextEnum(&protect, GrProtected::kYes);
#endif
    SkSurfaceCharacterization c;
    size_t maxResourceBytes = dContext->getResourceCacheLimit();
    c = dContext->threadSafeProxy()->createCharacterization(
                                maxResourceBytes, ii, backendFormat, kSampleCount,
                                origin, gen_fuzzed_surface_props(fuzz), true,
                                false, true, protect);
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
    canvas->drawRect(tile, gen_fuzzed_skpaint(fuzz));
    return r.detach();
}

static sk_sp<SkSurface> make_surface(Fuzz* fuzz, GrDirectContext* dContext, const SkImageInfo& ii,
                                     GrSurfaceOrigin origin) {
    skgpu::Budgeted budgeted;
    fuzz->nextEnum(&budgeted, skgpu::Budgeted::kYes);
    SkSurfaceProps surfaceProps = gen_fuzzed_surface_props(fuzz);
    auto surface = SkSurface::MakeRenderTarget(dContext, budgeted, ii, kSampleCount, origin,
                                               &surfaceProps);
    return surface;
}

static bool draw_ddl(sk_sp<SkSurface> surface, sk_sp<SkDeferredDisplayList> ddl) {
    return surface->draw(std::move(ddl));
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
    auto ctxInfo = factory.getContextInfo(sk_gpu_test::GrContextFactory::kGL_ContextType);

    GrDirectContext* dContext = ctxInfo.directContext();
    if (!dContext) {
        SkDebugf("Context creation failed");
        return;
    }

    auto[surface, c] = create_surface_and_characterization(fuzz, dContext, surfaceType, origin);
    if (!surface || !c.isValid()) {
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
