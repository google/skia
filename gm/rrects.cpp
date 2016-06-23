/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContextPriv.h"
#include "batches/GrDrawBatch.h"
#include "batches/GrRectBatchFactory.h"
#include "effects/GrRRectEffect.h"
#endif
#include "SkRRect.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class RRectGM : public GM {
public:
    enum Type {
        kBW_Draw_Type,
        kAA_Draw_Type,
        kBW_Clip_Type,
        kAA_Clip_Type,
        kEffect_Type,
    };
    RRectGM(Type type) : fType(type) { }

protected:

    void onOnceBeforeDraw() override {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        this->setUpRRects();
    }

    SkString onShortName() override {
        SkString name("rrect");
        switch (fType) {
            case kBW_Draw_Type:
                name.append("_draw_bw");
                break;
            case kAA_Draw_Type:
                name.append("_draw_aa");
                break;
            case kBW_Clip_Type:
                name.append("_clip_bw");
                break;
            case kAA_Clip_Type:
                name.append("_clip_aa");
                break;
            case kEffect_Type:
                name.append("_effect");
                break;
        }
        return name;
    }

    SkISize onISize() override { return SkISize::Make(kImageWidth, kImageHeight); }

    void onDraw(SkCanvas* canvas) override {
        GrDrawContext* drawContext = canvas->internal_private_accessTopLayerDrawContext();
        if (kEffect_Type == fType && !drawContext) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkPaint paint;
        if (kAA_Draw_Type == fType) {
            paint.setAntiAlias(true);
        }

        static const SkRect kMaxTileBound = SkRect::MakeWH(SkIntToScalar(kTileX),
                                                           SkIntToScalar(kTileY));
#ifdef SK_DEBUG
        static const SkRect kMaxImageBound = SkRect::MakeWH(SkIntToScalar(kImageWidth),
                                                            SkIntToScalar(kImageHeight));
#endif

#if SK_SUPPORT_GPU
        int lastEdgeType = (kEffect_Type == fType) ? kLast_GrProcessorEdgeType: 0;
#else
        int lastEdgeType = 0;
#endif

        int y = 1;
        for (int et = 0; et <= lastEdgeType; ++et) {
            int x = 1;
            for (int curRRect = 0; curRRect < kNumRRects; ++curRRect) {
                bool drew = true;
#ifdef SK_DEBUG
                SkASSERT(kMaxTileBound.contains(fRRects[curRRect].getBounds()));
                SkRect imageSpaceBounds = fRRects[curRRect].getBounds();
                imageSpaceBounds.offset(SkIntToScalar(x), SkIntToScalar(y));
                SkASSERT(kMaxImageBound.contains(imageSpaceBounds));
#endif
                canvas->save();
                    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                    if (kEffect_Type == fType) {
#if SK_SUPPORT_GPU
                        GrPaint grPaint;
                        grPaint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

                        SkRRect rrect = fRRects[curRRect];
                        rrect.offset(SkIntToScalar(x), SkIntToScalar(y));
                        GrPrimitiveEdgeType edgeType = (GrPrimitiveEdgeType) et;
                        sk_sp<GrFragmentProcessor> fp(GrRRectEffect::Make(edgeType, rrect));
                        if (fp) {
                            grPaint.addCoverageFragmentProcessor(std::move(fp));

                            SkRect bounds = rrect.getBounds();
                            bounds.outset(2.f, 2.f);

                            SkAutoTUnref<GrDrawBatch> batch(
                                    GrRectBatchFactory::CreateNonAAFill(0xff000000, SkMatrix::I(),
                                                                        bounds, nullptr, nullptr));
                            drawContext->drawContextPriv().testingOnly_drawBatch(grPaint, batch);
                        } else {
                            drew = false;
                        }
#endif
                    } else if (kBW_Clip_Type == fType || kAA_Clip_Type == fType) {
                        bool aaClip = (kAA_Clip_Type == fType);
                        canvas->clipRRect(fRRects[curRRect], SkRegion::kReplace_Op, aaClip);
                        canvas->drawRect(kMaxTileBound, paint);
                    } else {
                        canvas->drawRRect(fRRects[curRRect], paint);
                    }
                canvas->restore();
                if (drew) {
                    x = x + kTileX;
                    if (x > kImageWidth) {
                        x = 1;
                        y += kTileY;
                    }
                }
            }
            if (x != 1) {
                y += kTileY;
            }
        }
    }

    void setUpRRects() {
        // each RRect must fit in a 0x0 -> (kTileX-2)x(kTileY-2) block. These will be tiled across
        // the screen in kTileX x kTileY tiles. The extra empty pixels on each side are for AA.

        // simple cases
        fRRects[0].setRect(SkRect::MakeWH(kTileX-2, kTileY-2));
        fRRects[1].setOval(SkRect::MakeWH(kTileX-2, kTileY-2));
        fRRects[2].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 10, 10);
        fRRects[3].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 10, 5);
        // small circular corners are an interesting test case for gpu clipping
        fRRects[4].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 1, 1);
        fRRects[5].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 0.5f, 0.5f);
        fRRects[6].setRectXY(SkRect::MakeWH(kTileX-2, kTileY-2), 0.2f, 0.2f);

        // The first complex case needs special handling since it is a square
        fRRects[kNumSimpleCases].setRectRadii(SkRect::MakeWH(kTileY-2, kTileY-2), gRadii[0]);
        for (size_t i = 1; i < SK_ARRAY_COUNT(gRadii); ++i) {
            fRRects[kNumSimpleCases+i].setRectRadii(SkRect::MakeWH(kTileX-2, kTileY-2), gRadii[i]);
        }
    }

private:
    Type fType;

    static const int kImageWidth = 640;
    static const int kImageHeight = 480;

    static const int kTileX = 80;
    static const int kTileY = 40;

    static const int kNumSimpleCases = 7;
    static const int kNumComplexCases = 35;
    static const SkVector gRadii[kNumComplexCases][4];

    static const int kNumRRects = kNumSimpleCases + kNumComplexCases;
    SkRRect fRRects[kNumRRects];

    typedef GM INHERITED;
};

// Radii for the various test cases. Order is UL, UR, LR, LL
const SkVector RRectGM::gRadii[kNumComplexCases][4] = {
    // a circle
    { { kTileY, kTileY }, { kTileY, kTileY }, { kTileY, kTileY }, { kTileY, kTileY } },

    // odd ball cases
    { { 8, 8 }, { 32, 32 }, { 8, 8 }, { 32, 32 } },
    { { 16, 8 }, { 8, 16 }, { 16, 8 }, { 8, 16 } },
    { { 0, 0 }, { 16, 16 }, { 8, 8 }, { 32, 32 } },

    // UL
    { { 30, 30 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 30, 15 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 15, 30 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },

    // UR
    { { 0, 0 }, { 30, 30 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 30, 15 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 15, 30 }, { 0, 0 }, { 0, 0 } },

    // LR
    { { 0, 0 }, { 0, 0 }, { 30, 30 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 30, 15 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 15, 30 }, { 0, 0 } },

    // LL
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 30, 30 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 30, 15 } },
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 15, 30 } },

    // over-sized radii
    { { 0, 0 }, { 100, 400 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 400, 400 }, { 0, 0 }, { 0, 0 } },
    { { 400, 400 }, { 400, 400 }, { 400, 400 }, { 400, 400 } },

    // circular corner tabs
    { { 0, 0 }, { 20, 20 }, { 20, 20 }, { 0, 0 } },
    { { 20, 20 }, { 20, 20 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 0, 0 }, { 20, 20 }, { 20, 20 } },
    { { 20, 20 }, { 0, 0 }, { 0, 0 }, { 20, 20 } },

    // small radius circular corner tabs
    { { 0, 0 }, { 0.2f, 0.2f }, { 0.2f, 0.2f }, { 0, 0 } },
    { { 0.3f, 0.3f }, { 0.3f, .3f }, { 0, 0 }, { 0, 0 } },

    // single circular corner cases
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 15, 15 } },
    { { 0, 0 }, { 0, 0 }, { 15, 15 }, { 0, 0 } },
    { { 0, 0 }, { 15, 15 }, { 0, 0 }, { 0, 0 } },
    { { 15, 15 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },

    // nine patch elliptical
    { { 5, 7 }, { 8, 7 }, { 8, 12 }, { 5, 12 } },
    { { 0, 7 }, { 8, 7 }, { 8, 12 }, { 0, 12 } },

    // nine patch elliptical, small radii
    { { 0.4f, 7 }, { 8, 7 }, { 8, 12 }, { 0.4f, 12 } },
    { { 0.4f, 0.4f }, { 8, 0.4f }, { 8, 12 }, { 0.4f, 12 } },
    { { 20, 0.4f }, { 18, 0.4f }, { 18, 0.4f }, { 20, 0.4f } },
    { { 0.3f, 0.4f }, { 0.3f, 0.4f }, { 0.3f, 0.4f }, { 0.3f, 0.4f } },

};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new RRectGM(RRectGM::kAA_Draw_Type); )
DEF_GM( return new RRectGM(RRectGM::kBW_Draw_Type); )
DEF_GM( return new RRectGM(RRectGM::kAA_Clip_Type); )
DEF_GM( return new RRectGM(RRectGM::kBW_Clip_Type); )
#if SK_SUPPORT_GPU
DEF_GM( return new RRectGM(RRectGM::kEffect_Type); )
#endif

}
