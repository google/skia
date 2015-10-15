
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
#include "GrDrawContext.h"
#include "SkColorPriv.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

static const int S = 200;

DEF_SIMPLE_GM_BG(texdata, canvas, 2 * S, 2 * S, SK_ColorBLACK) {
        GrRenderTarget* target = canvas->internal_private_accessTopLayerRenderTarget();
        GrContext* ctx = canvas->getGrContext();
        SkAutoTUnref<GrDrawContext> drawContext(ctx ? ctx->drawContext(target) : nullptr);
        if (drawContext && target) {
            SkAutoTArray<SkPMColor> gTextureData((2 * S) * (2 * S));
            static const int stride = 2 * S;
            static const SkPMColor gray  = SkPackARGB32(0x40, 0x40, 0x40, 0x40);
            static const SkPMColor white = SkPackARGB32(0xff, 0xff, 0xff, 0xff);
            static const SkPMColor red   = SkPackARGB32(0x80, 0x80, 0x00, 0x00);
            static const SkPMColor blue  = SkPackARGB32(0x80, 0x00, 0x00, 0x80);
            static const SkPMColor green = SkPackARGB32(0x80, 0x00, 0x80, 0x00);
            static const SkPMColor black = SkPackARGB32(0x00, 0x00, 0x00, 0x00);
            for (int i = 0; i < 2; ++i) {
                int offset = 0;
                // fill upper-left
                for (int y = 0; y < S; ++y) {
                    for (int x = 0; x < S; ++x) {
                        gTextureData[offset + y * stride + x] = gray;
                    }
                }
                // fill upper-right
                offset = S;
                for (int y = 0; y < S; ++y) {
                    for (int x = 0; x < S; ++x) {
                        gTextureData[offset + y * stride + x] = white;
                    }
                }
                // fill lower left
                offset = S * stride;
                for (int y = 0; y < S; ++y) {
                    for (int x = 0; x < S; ++x) {
                        gTextureData[offset + y * stride + x] = black;
                    }
                }
                // fill lower right
                offset = S * stride + S;
                for (int y = 0; y < S; ++y) {
                    for (int x = 0; x < S; ++x) {
                        gTextureData[offset + y * stride + x] = gray;
                    }
                }

                GrSurfaceDesc desc;
                // use RT flag bit because in GL it makes the texture be bottom-up
                desc.fFlags     = i ? kRenderTarget_GrSurfaceFlag :
                                      kNone_GrSurfaceFlags;
                desc.fConfig    = kSkia8888_GrPixelConfig;
                desc.fWidth     = 2 * S;
                desc.fHeight    = 2 * S;
                GrTexture* texture = ctx->textureProvider()->createTexture(
                    desc, false, gTextureData.get(), 0);

                if (!texture) {
                    return;
                }
                SkAutoTUnref<GrTexture> au(texture);

                // setup new clip
                GrClip clip(SkRect::MakeWH(2*S, 2*S));

                GrPaint paint;
                paint.setPorterDuffXPFactory(SkXfermode::kSrcOver_Mode);

                SkMatrix vm;
                if (i) {
                    vm.setRotate(90 * SK_Scalar1,
                                 S * SK_Scalar1,
                                 S * SK_Scalar1);
                } else {
                    vm.reset();
                }
                SkMatrix tm;
                tm = vm;
                tm.postIDiv(2*S, 2*S);
                paint.addColorTextureProcessor(texture, tm);

                drawContext->drawRect(clip, paint, vm, SkRect::MakeWH(2*S, 2*S));

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
                drawContext->drawRect(clip, paint, vm, SkRect::MakeWH(2*S, 2*S));
            }
        } else {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
        }
}
#endif
