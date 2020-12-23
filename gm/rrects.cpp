/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/GrSharedEnums.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrFillRectOp.h"

#include <memory>
#include <utility>

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
        this->setBGColor(0xFFDDDDDD);
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

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        GrSurfaceDrawContext* surfaceDrawContext =
                SkCanvasPriv::TopDeviceSurfaceDrawContext(canvas);

        auto context = canvas->recordingContext();
        if (kEffect_Type == fType && (!surfaceDrawContext || !context)) {
            *errorMsg = kErrorMsg_DrawSkippedGpuOnly;
            return DrawResult::kSkip;
        }

        SkPaint paint;
        if (kAA_Draw_Type == fType) {
            paint.setAntiAlias(true);
        }

        const SkRect kMaxTileBound = SkRect::MakeWH(SkIntToScalar(kTileX),
                                                     SkIntToScalar(kTileY));
#ifdef SK_DEBUG
        const SkRect kMaxImageBound = SkRect::MakeWH(SkIntToScalar(kImageWidth),
                                                     SkIntToScalar(kImageHeight));
#endif

        int lastEdgeType = (kEffect_Type == fType) ? (int) GrClipEdgeType::kLast: 0;

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
                        SkRRect rrect = fRRects[curRRect];
                        rrect.offset(SkIntToScalar(x), SkIntToScalar(y));
                        GrClipEdgeType edgeType = (GrClipEdgeType) et;
                        const auto& caps = *surfaceDrawContext->caps()->shaderCaps();
                        auto [success, fp] = GrRRectEffect::Make(/*inputFP=*/nullptr,
                                                                 edgeType, rrect, caps);
                        if (success) {
                            GrPaint grPaint;
                            grPaint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
                            grPaint.setCoverageFragmentProcessor(std::move(fp));
                            grPaint.setColor4f({ 0, 0, 0, 1.f });

                            SkRect bounds = rrect.getBounds();
                            bounds.outset(2.f, 2.f);

                            surfaceDrawContext->addDrawOp(GrFillRectOp::MakeNonAARect(
                                    context, std::move(grPaint), SkMatrix::I(), bounds));
                        } else {
                            drew = false;
                        }
                    } else if (kBW_Clip_Type == fType || kAA_Clip_Type == fType) {
                        bool aaClip = (kAA_Clip_Type == fType);
                        canvas->clipRRect(fRRects[curRRect], aaClip);
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
        return DrawResult::kOk;
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

    static constexpr int kImageWidth = 640;
    static constexpr int kImageHeight = 480;

    static constexpr int kTileX = 80;
    static constexpr int kTileY = 40;

    static constexpr int kNumSimpleCases = 7;
    static constexpr int kNumComplexCases = 35;
    static const SkVector gRadii[kNumComplexCases][4];

    static constexpr int kNumRRects = kNumSimpleCases + kNumComplexCases;
    SkRRect fRRects[kNumRRects];

    using INHERITED = GM;
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
DEF_GM( return new RRectGM(RRectGM::kEffect_Type); )

}  // namespace skiagm
