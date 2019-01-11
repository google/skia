/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"

#if SK_SUPPORT_GPU
#include "etc1.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTextureProxy.h"
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrFillRectOp.h"

// Basic test of Ganesh's ETC1 support
class ETC1GM : public skiagm::GM {
public:
    ETC1GM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("etc1");
    }

    SkISize onISize() override {
        return SkISize::Make(kTexWidth + 2*kPad, kTexHeight + 2*kPad);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bm;
        SkImageInfo ii = SkImageInfo::Make(kTexWidth, kTexHeight, kRGB_565_SkColorType,
                                           kOpaque_SkAlphaType);
        bm.allocPixels(ii);

        bm.erase(SK_ColorBLUE, SkIRect::MakeWH(kTexWidth, kTexHeight));

        for (int y = 0; y < kTexHeight; y += 4) {
            for (int x = 0; x < kTexWidth; x += 4) {
                bm.erase((x+y) % 8 ? SK_ColorRED : SK_ColorGREEN, SkIRect::MakeXYWH(x, y, 4, 4));
            }
        }

        int size = etc1_get_encoded_data_size(bm.width(), bm.height());
        fETC1Data.reset(size);

        unsigned char* pixels = (unsigned char*) fETC1Data.get();

        if (etc1_encode_image((unsigned char*) bm.getAddr16(0, 0),
                              bm.width(), bm.height(), 2, bm.rowBytes(), pixels)) {
            fETC1Data.reset();
        }
    }

    void onDraw(SkCanvas* canvas) override {
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

        GrBackendTexture tex = context->contextPriv().getGpu()->createTestingOnlyBackendTexture(
            fETC1Data.get(),
            kTexWidth,
            kTexHeight,
            GrColorType::kRGB_ETC1,
            false,
            GrMipMapped::kNo,
            kTexWidth/2); // rowbytes are meaningless for compressed textures, but this is
                          // basically right

        if (!tex.isValid()) {
            return;
        }

        auto proxy = context->contextPriv().proxyProvider()->wrapBackendTexture(
                                                                 tex, kTopLeft_GrSurfaceOrigin,
                                                                 kAdopt_GrWrapOwnership,
                                                                 kRead_GrIOType);
        if (!proxy) {
            return;
        }

        const SkMatrix trans = SkMatrix::MakeTrans(-kPad, -kPad);

        auto fp = GrSimpleTextureEffect::Make(proxy, trans);

        GrPaint grPaint;
        grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
        grPaint.addColorFragmentProcessor(std::move(fp));

        SkRect rect = SkRect::MakeXYWH(kPad, kPad, kTexWidth, kTexHeight);

        renderTargetContext->priv().testingOnly_addDrawOp(
            GrFillRectOp::Make(context, std::move(grPaint), GrAAType::kNone, SkMatrix::I(), rect));
    }

private:
    static const int kPad = 8;
    static const int kTexWidth = 16;
    static const int kTexHeight = 20;

    SkAutoTMalloc<char> fETC1Data;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ETC1GM;)

#endif
