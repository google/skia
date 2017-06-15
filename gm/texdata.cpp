/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This test only works with the GPU backend.

#include "gm.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrTextureContext.h"
#include "GrFixedClip.h"
#include "SkColorPriv.h"
#include "SkGr.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

constexpr int S = 200;
constexpr int kStride = 2 * S;

// Fill in the pixels:
//   gray  | white
//   -------------
//   black | gray
static void fill_in_pixels(SkPMColor* pixels) {
    const SkPMColor gray  = SkPackARGB32(0x40, 0x40, 0x40, 0x40);
    const SkPMColor white = SkPackARGB32(0xff, 0xff, 0xff, 0xff);
    const SkPMColor black = SkPackARGB32(0x00, 0x00, 0x00, 0x00);

    int offset = 0;

    // fill upper-left
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            pixels[offset + y * kStride + x] = gray;
        }
    }
    // fill upper-right
    offset = S;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            pixels[offset + y * kStride + x] = white;
        }
    }
    // fill lower left
    offset = S * kStride;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            pixels[offset + y * kStride + x] = black;
        }
    }
    // fill lower right
    offset = S * kStride + S;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            pixels[offset + y * kStride + x] = gray;
        }
    }
}

DEF_SIMPLE_GM_BG(texdata, canvas, 2 * S, 2 * S, SK_ColorBLACK) {
    GrRenderTargetContext* renderTargetContext =
        canvas->internal_private_accessTopLayerRenderTargetContext();
    if (!renderTargetContext) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }

    GrContext* context = canvas->getGrContext();
    if (!context) {
        return;
    }

    const SkImageInfo ii = SkImageInfo::Make(S, S, kBGRA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoTArray<SkPMColor> gTextureData((2 * S) * (2 * S));
    const SkPMColor red   = SkPackARGB32(0x80, 0x80, 0x00, 0x00);
    const SkPMColor blue  = SkPackARGB32(0x80, 0x00, 0x00, 0x80);
    const SkPMColor green = SkPackARGB32(0x80, 0x00, 0x80, 0x00);
    for (int i = 0; i < 2; ++i) {
        fill_in_pixels(gTextureData.get());

        GrSurfaceDesc desc;
        desc.fOrigin    = i ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
        desc.fWidth     = 2 * S;
        desc.fHeight    = 2 * S;
        desc.fConfig    = SkImageInfo2GrPixelConfig(ii, *context->caps());

        sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                   desc, SkBudgeted::kNo,
                                                                   gTextureData.get(), 0);
        if (!proxy) {
            return;
        }

        sk_sp<GrSurfaceContext> tContext = context->contextPriv().makeWrappedSurfaceContext(
                std::move(proxy), nullptr);

        if (!tContext) {
            return;
        }

        // setup new clip
        GrFixedClip clip(SkIRect::MakeWH(2*S, 2*S));

        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);

        SkMatrix vm;
        if (i) {
            vm.setRotate(90 * SK_Scalar1, S * SK_Scalar1, S * SK_Scalar1);
        } else {
            vm.reset();
        }
        paint.addColorTextureProcessor(tContext->asTextureProxyRef(), nullptr, vm);

        renderTargetContext->drawRect(clip, GrPaint(paint), GrAA::kNo, vm,
                                      SkRect::MakeWH(2 * S, 2 * S));

        // now update the lower right of the texture in first pass
        // or upper right in second pass
        for (int y = 0; y < S; ++y) {
            for (int x = 0; x < S; ++x) {
                gTextureData[y * kStride + x] = ((x + y) % 2) ? (i ? green : red) : blue;
            }
        }

        if (!tContext->writePixels(ii, gTextureData.get(), 4 * kStride, S, i ? 0 : S)) {
            continue;
        }

        renderTargetContext->drawRect(clip, std::move(paint), GrAA::kNo, vm,
                                      SkRect::MakeWH(2 * S, 2 * S));
    }
}
#endif

