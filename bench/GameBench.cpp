/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"


// This bench simulates the calls Skia sees from various HTML5 canvas
// game bench marks
class GameBench : public SkBenchmark {
public:
    enum Type {
        kScale_Type,
        kTranslate_Type,
        kRotate_Type
    };

    GameBench(void* param, Type type, bool partialClear)
        : INHERITED(param)
        , fType(type)
        , fPartialClear(partialClear)
        , fName("game")
        , fNumSaved(0)
        , fInitialized(false) {

        switch (fType) {
        case kScale_Type:
            fName.append("_scale");
            break;
        case kTranslate_Type:
            fName.append("_trans");
            break;
        case kRotate_Type:
            fName.append("_rot");
            break;
        };

        if (partialClear) {
            fName.append("_partial");
        } else {
            fName.append("_full");
        }

        // It's HTML 5 canvas, so always AA
        fName.append("_aa");
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();;
    }

    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            this->makeCheckerboard();
            fInitialized = true;
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        static SkMWCRandom scaleRand;
        static SkMWCRandom transRand;
        static SkMWCRandom rotRand;

        SkPaint clearPaint;
        clearPaint.setColor(0xFF000000);
        clearPaint.setAntiAlias(true);

        SkISize size = canvas->getDeviceSize();

        SkScalar maxTransX, maxTransY;

        if (kScale_Type == fType) {
            maxTransX = size.fWidth  - (1.5f * kCheckerboardWidth);
            maxTransY = size.fHeight - (1.5f * kCheckerboardHeight);
        } else if (kTranslate_Type == fType) {
            maxTransX = SkIntToScalar(size.fWidth  - kCheckerboardWidth);
            maxTransY = SkIntToScalar(size.fHeight - kCheckerboardHeight);
        } else {
            SkASSERT(kRotate_Type == fType);
            // Yes, some rotations will be off the top and left sides
            maxTransX = size.fWidth  - SK_ScalarSqrt2 * kCheckerboardHeight;
            maxTransY = size.fHeight - SK_ScalarSqrt2 * kCheckerboardHeight;
        }

        SkMatrix mat;
        SkIRect src = { 0, 0, kCheckerboardWidth, kCheckerboardHeight };
        SkRect dst = { 0, 0, kCheckerboardWidth, kCheckerboardHeight };
        SkRect clearRect = { -1.0f, -1.0f,
                             kCheckerboardWidth+1.0f, kCheckerboardHeight+1.0f };

        SkPaint p;
        p.setColor(0xFF000000);
        p.setFilterBitmap(true);

        for (int i = 0; i < kNumRects; ++i, ++fNumSaved) {

            if (0 == i % kNumBeforeClear) {
                if (fPartialClear) {
                    for (int j = 0; j < fNumSaved; ++j) {
                        canvas->setMatrix(SkMatrix::I());
                        mat.setTranslate(fSaved[j][0], fSaved[j][1]);

                        if (kScale_Type == fType) {
                            mat.preScale(fSaved[j][2], fSaved[j][2]);
                        } else if (kRotate_Type == fType) {
                            mat.preRotate(fSaved[j][2]);
                        }

                        canvas->concat(mat);
                        canvas->drawRect(clearRect, clearPaint);
                    }
                } else {
                    canvas->clear(0xFF000000);
                }

                fNumSaved = 0;
            }

            SkASSERT(fNumSaved < kNumBeforeClear);

            canvas->setMatrix(SkMatrix::I());

            fSaved[fNumSaved][0] = transRand.nextRangeScalar(0.0f, maxTransX);
            fSaved[fNumSaved][1] = transRand.nextRangeScalar(0.0f, maxTransY);

            mat.setTranslate(fSaved[fNumSaved][0], fSaved[fNumSaved][1]);

            if (kScale_Type == fType) {
                fSaved[fNumSaved][2] = scaleRand.nextRangeScalar(0.5f, 1.5f);
                mat.preScale(fSaved[fNumSaved][2], fSaved[fNumSaved][2]);
            } else if (kRotate_Type == fType) {
                fSaved[fNumSaved][2] = rotRand.nextRangeScalar(0.0f, 360.0f);
                mat.preRotate(fSaved[fNumSaved][2]);
            }

            canvas->concat(mat);
            canvas->drawBitmapRect(fCheckerboard, &src, dst, &p);
        }
    }

private:
    static const int kCheckerboardWidth = 64;
    static const int kCheckerboardHeight = 128;
#ifdef SK_DEBUG
    static const int kNumRects = 100;
    static const int kNumBeforeClear = 10;
#else
    static const int kNumRects = 5000;
    static const int kNumBeforeClear = 300;
#endif


    Type     fType;
    bool     fPartialClear;
    SkString fName;
    int      fNumSaved; // num draws stored in 'fSaved'
    bool     fInitialized;

    // 0 & 1 are always x & y translate. 2 is either scale or rotate.
    SkScalar fSaved[kNumBeforeClear][3];
    SkBitmap fCheckerboard;

    // Note: the resulting checker board has transparency
    void makeCheckerboard() {
        static int kCheckSize = 16;

        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config,
                                kCheckerboardWidth, kCheckerboardHeight);
        fCheckerboard.allocPixels();
        SkAutoLockPixels lock(fCheckerboard);
        for (int y = 0; y < kCheckerboardHeight; ++y) {
            int even = (y / kCheckSize) % 2;

            SkPMColor* scanline = fCheckerboard.getAddr32(0, y);

            for (int x = 0; x < kCheckerboardWidth; ++x) {
                if (even == (x / kCheckSize) % 2) {
                    *scanline++ = 0xFFFF0000;
                } else {
                    *scanline++ = 0x00000000;
                }
            }
        }
    }

    typedef SkBenchmark INHERITED;
};

// Partial clear
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kScale_Type, false)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kTranslate_Type, false)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kRotate_Type, false)); )

// Full clear
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kScale_Type, true)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kTranslate_Type, true)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (p, GameBench::kRotate_Type, true)); )
