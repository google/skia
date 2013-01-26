/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRRect.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

class RRectGM : public GM {
public:
    RRectGM(bool doAA, bool doClip) : fDoAA(doAA), fDoClip(doClip) {
        this->setBGColor(0xFFDDDDDD);
        this->setUpRRects();
    }

protected:
    SkString onShortName() {
        SkString name("rrect");
        if (fDoClip) {
            name.append("_clip");
        }
        if (fDoAA) {
            name.append("_aa");
        } else {
            name.append("_bw");
        }

        return name;
    }

    virtual SkISize onISize() { return make_isize(kImageWidth, kImageHeight); }

    virtual void onDraw(SkCanvas* canvas) {

        SkPaint paint;
        // when clipping the AA is pushed into the clip operation
        paint.setAntiAlias(fDoClip ? false : fDoAA);

        static const SkRect kMaxTileBound = SkRect::MakeWH(SkIntToScalar(kTileX), SkIntToScalar(kTileY));

        int curRRect = 0;
        for (int y = 1; y < kImageHeight; y += kTileY) {
            for (int x = 1; x < kImageWidth; x += kTileX) {
                if (curRRect >= kNumRRects) {
                    break;
                }
                SkASSERT(kMaxTileBound.contains(fRRects[curRRect].getBounds()));

                canvas->save();
                    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
                    if (fDoClip) {
                        canvas->clipRRect(fRRects[curRRect], SkRegion::kReplace_Op, fDoAA);
                        canvas->drawRect(kMaxTileBound, paint);
                    } else {
                        canvas->drawRRect(fRRects[curRRect], paint);
                    }
                    ++curRRect;
                canvas->restore();
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

        // The first complex case needs special handling since it is a square
        fRRects[kNumSimpleCases].setRectRadii(SkRect::MakeWH(kTileY-2, kTileY-2), gRadii[0]);
        for (size_t i = 1; i < SK_ARRAY_COUNT(gRadii); ++i) {
            fRRects[kNumSimpleCases+i].setRectRadii(SkRect::MakeWH(kTileX-2, kTileY-2), gRadii[i]);
        }
    }

private:
    bool fDoAA;
    bool fDoClip;   // use clipRRect & drawRect instead of drawRRect

    static const int kImageWidth = 640;
    static const int kImageHeight = 480;

    static const int kTileX = 80;
    static const int kTileY = 40;

    static const int kNumSimpleCases = 3;
    static const int kNumComplexCases = 19;
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
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new RRectGM(false, false); )
DEF_GM( return new RRectGM(true, false); )
DEF_GM( return new RRectGM(false, true); )
DEF_GM( return new RRectGM(true, true); )

}
