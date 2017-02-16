/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

// Make 4 SkBitmaps that each point to a different colored tile w/in a single
// SkPixelRef.
static void make_subset_bitmaps(SkBitmap* red, SkBitmap* green, SkBitmap* blue, SkBitmap* cyan) {
    static const int kPad = 8;
    static const int kTileSize = 16;
    static const int kTotSize = 3*kPad + 2 * kTileSize;
    static const int kFarStart = 2*kPad + kTileSize;

    SkBitmap master;
    master.allocN32Pixels(kTotSize, kTotSize, true);

    static const SkIRect redRect   = SkIRect::MakeXYWH(kPad,      kPad,      kTileSize, kTileSize);
    static const SkIRect greenRect = SkIRect::MakeXYWH(kFarStart, kPad,      kTileSize, kTileSize);
    static const SkIRect blueRect  = SkIRect::MakeXYWH(kPad,      kFarStart, kTileSize, kTileSize);
    static const SkIRect cyanRect  = SkIRect::MakeXYWH(kFarStart, kFarStart, kTileSize, kTileSize);

    master.erase(SK_ColorBLACK, SkIRect::MakeWH(kTileSize, kTileSize));
    master.erase(SK_ColorRED,   redRect);
    master.erase(SK_ColorGREEN, greenRect);
    master.erase(SK_ColorBLUE,  blueRect);
    master.erase(SK_ColorCYAN,  cyanRect);

    master.extractSubset(red,   redRect);
    master.extractSubset(green, greenRect);
    master.extractSubset(blue,  blueRect);
    master.extractSubset(cyan,  cyanRect);
}


class SubsetBitmapsGM : public skiagm::GM {
public:
    SubsetBitmapsGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("subsetbitmaps");
    }

    SkISize onISize() override {
        return SkISize::Make(100, 100);
    }

    void onOnceBeforeDraw() override {
        make_subset_bitmaps(&fRedBM, &fGreenBM, &fBlueBM, &fCyanBM);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawBitmap(fRedBM, 10, 10);
        canvas->drawBitmap(fGreenBM, 50, 10);
        canvas->drawBitmap(fBlueBM, 10, 50);
        canvas->drawBitmap(fCyanBM, 50, 50);
    }

private:
    SkBitmap fRedBM;
    SkBitmap fGreenBM;
    SkBitmap fBlueBM;
    SkBitmap fCyanBM;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SubsetBitmapsGM;)
