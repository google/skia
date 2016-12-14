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
#include "GrRenderTargetContext.h"
#include "GrFixedClip.h"
#include "SkColorPriv.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

constexpr int S = 200;

// Gray  White
// Black Gray
static void create_texture_data(SkPMColor* data) {
    constexpr int stride = 2 * S;
    const SkPMColor gray  = SkPackARGB32(0x40, 0x40, 0x40, 0x40);
    const SkPMColor white = SkPackARGB32(0xff, 0xff, 0xff, 0xff);
    const SkPMColor red   = SkPackARGB32(0x80, 0x80, 0x00, 0x00);
    const SkPMColor blue  = SkPackARGB32(0x80, 0x00, 0x00, 0x80);
    const SkPMColor green = SkPackARGB32(0x80, 0x00, 0x80, 0x00);
    const SkPMColor black = SkPackARGB32(0x00, 0x00, 0x00, 0x00);

    int offset = 0;
    // fill upper-left
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            data[offset + y * stride + x] = gray;
        }
    }
    // fill upper-right
    offset = S;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            data[offset + y * stride + x] = white;
        }
    }
    // fill lower left
    offset = S * stride;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            data[offset + y * stride + x] = black;
        }
    }
    // fill lower right
    offset = S * stride + S;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            data[offset + y * stride + x] = gray;
        }
    }
}

#include "GrTextureProxy.h"

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

    SkAutoTArray<SkPMColor> gTextureData((2 * S) * (2 * S));
    for (int i = 0; i < 2; ++i) {
        create_texture_data(gTextureData.get());

        GrSurfaceDesc desc;
        desc.fOrigin    = i ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
        desc.fConfig    = kSkia8888_GrPixelConfig;
        desc.fWidth     = 2 * S;
        desc.fHeight    = 2 * S;

        sk_sp<GrSurfaceProxy> tProxy = GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                                    context->textureProvider(),
                                                                    desc,
                                                                    SkBudgeted::kNo,
                                                                    gTextureData.get(), 0);
        if (!tProxy) {
            return;
        }

#if 0
        GrTexture* texture = context->textureProvider()->createTexture(
            desc, SkBudgeted::kNo, gTextureData.get(), 0);

        if (!texture) {
            return;
        }
        sk_sp<GrTexture> au(texture);
#endif

#if 0
        // setup new clip
        GrFixedClip clip(SkIRect::MakeWH(2*S, 2*S));

        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrcOver);

        SkMatrix vm;
        if (i) {
            vm.setRotate(90.0f, SkIntToScalar(S), SkIntToScalar(S));
        } else {
            vm.reset();
        }
        SkMatrix tm;
        tm = vm;
        tm.postIDiv(2*S, 2*S);
        paint.addColorTextureProcessor(texture, nullptr, tm);

        renderTargetContext->drawRect(clip, paint, GrAA::kNo, vm, SkRect::MakeWH(2*S, 2*S));

        // now update the lower right of the texture in first pass
        // or upper right in second pass
        offset = 0;
        for (int y = 0; y < S; ++y) {
            for (int x = 0; x < S; ++x) {
                gTextureData[offset + y * stride + x] =
                    ((x + y) % 2) ? (i ? green : red) : blue;
            }
        }
        texture->writePixels(S, (i ? 0 : S), S, S,
                                texture->config(), gTextureData.get(),
                                4 * stride);
        renderTargetContext->drawRect(clip, paint, GrAA::kNo, vm, SkRect::MakeWH(2*S, 2*S));
    }
#endif
    }
}
#endif

