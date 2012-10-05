
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
#include "effects/GrSingleTextureEffect.h"
#include "SkColorPriv.h"
#include "SkDevice.h"

namespace skiagm {

extern GrContext* GetGr();

static const int S = 200;

class TexDataGM : public GM {
public:
    TexDataGM() {
        this->setBGColor(0xff000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("texdata");
    }

    virtual SkISize onISize() {
        return make_isize(2*S, 2*S);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkDevice* device = canvas->getDevice();
        GrRenderTarget* target = (GrRenderTarget*) device->accessRenderTarget();
        GrContext* ctx = GetGr();
        if (ctx && target) {
            SkPMColor gTextureData[(2 * S) * (2 * S)];
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

                GrTextureDesc desc;
                // use RT flag bit because in GL it makes the texture be bottom-up
                desc.fFlags     = i ? kRenderTarget_GrTextureFlagBit :
                                      kNone_GrTextureFlags;
                desc.fConfig    = kSkia8888_PM_GrPixelConfig;
                desc.fWidth     = 2 * S;
                desc.fHeight    = 2 * S;
                GrTexture* texture =
                    ctx->createUncachedTexture(desc, gTextureData, 0);

                if (!texture) {
                    return;
                }
                GrAutoUnref au(texture);

                GrContext::AutoClip acs(ctx, GrRect::MakeWH(2*S, 2*S));

                ctx->setRenderTarget(target);

                GrPaint paint;
                paint.reset();
                paint.fColor = 0xffffffff;
                paint.fSrcBlendCoeff = kOne_GrBlendCoeff;
                paint.fDstBlendCoeff = kISA_GrBlendCoeff;
                GrMatrix vm;
                if (i) {
                    vm.setRotate(90 * SK_Scalar1,
                                 S * SK_Scalar1,
                                 S * SK_Scalar1);
                } else {
                    vm.reset();
                }
                ctx->setMatrix(vm);
                GrMatrix tm;
                tm = vm;
                GrMatrix* sampleMat = paint.colorSampler(0)->matrix();
                *sampleMat = vm;
                sampleMat->postIDiv(2*S, 2*S);
                paint.colorSampler(0)->setCustomStage(
                    SkNEW_ARGS(GrSingleTextureEffect, (texture)))->unref();


                ctx->drawRect(paint, GrRect::MakeWH(2*S, 2*S));

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
                                     texture->config(), gTextureData,
                                     4 * stride);
                ctx->drawRect(paint, GrRect::MakeWH(2*S, 2*S));
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new TexDataGM; }
static GMRegistry reg(MyFactory);

}

#endif
