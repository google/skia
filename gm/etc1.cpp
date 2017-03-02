/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRandom.h"

#if SK_SUPPORT_GPU
#include "etc1.h"

#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrTextureProxy.h"
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrRectOpFactory.h"

// Basic test of Ganesh's ETC1 support
class ETC1GM : public skiagm::GM {
public:
    ETC1GM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    SkString onShortName() override {
        return SkString("etc1");
    }

    SkISize onISize() override {
        return SkISize::Make(kTexWidth + 2*kPad, kTexHeight + 2*kPad);
    }

    // TODO: we should be creating an ETC1 SkData blob here and going through SkImageCacherator.
    // That will require an ETC1 Codec though - so for later.
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

        GrSurfaceDesc desc;
        desc.fConfig = kETC1_GrPixelConfig;
        desc.fWidth = kTexWidth;
        desc.fHeight = kTexHeight;

        sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                                   context->textureProvider(),
                                                                   desc, SkBudgeted::kYes,
                                                                   fETC1Data.get(), 0);
        if (!proxy) {
            return;
        }

        const SkMatrix trans = SkMatrix::MakeTrans(-kPad, -kPad);

        sk_sp<GrFragmentProcessor> fp = GrSimpleTextureEffect::Make(context,
                                                                    std::move(proxy),
                                                                    nullptr, trans);

        GrPaint grPaint;
        grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
        grPaint.addColorFragmentProcessor(std::move(fp));

        SkRect rect = SkRect::MakeXYWH(kPad, kPad, kTexWidth, kTexHeight);

        std::unique_ptr<GrDrawOp> op(GrRectOpFactory::MakeNonAAFill(
                GrColor_WHITE, SkMatrix::I(), rect, nullptr, nullptr));
        renderTargetContext->priv().testingOnly_addDrawOp(
                std::move(grPaint), GrAAType::kNone, std::move(op));
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
