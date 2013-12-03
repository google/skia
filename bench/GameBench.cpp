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
#include "SkShader.h"
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

    enum Clear {
        kFull_Clear,
        kPartial_Clear
    };

    GameBench(Type type, Clear clear,
              bool aligned = false, bool useAtlas = false,
              bool useDrawVertices = false)
        : fType(type)
        , fClear(clear)
        , fAligned(aligned)
        , fUseAtlas(useAtlas)
        , fUseDrawVertices(useDrawVertices)
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

        if (aligned) {
            fName.append("_aligned");
        }

        if (kPartial_Clear == clear) {
            fName.append("_partial");
        } else {
            fName.append("_full");
        }

        if (useAtlas) {
            fName.append("_atlas");
        }

        if (useDrawVertices) {
            fName.append("_drawVerts");
        }

        // It's HTML 5 canvas, so always AA
        fName.append("_aa");
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onPreDraw() SK_OVERRIDE {
        if (!fInitialized) {
            this->makeCheckerboard();
            this->makeAtlas();
            fInitialized = true;
        }
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom scaleRand;
        SkRandom transRand;
        SkRandom rotRand;

        int width, height;
        if (fUseAtlas) {
            width = kAtlasCellWidth;
            height = kAtlasCellHeight;
        } else {
            width = kCheckerboardWidth;
            height = kCheckerboardHeight;
        }

        SkPaint clearPaint;
        clearPaint.setColor(0xFF000000);
        clearPaint.setAntiAlias(true);

        SkISize size = canvas->getDeviceSize();

        SkScalar maxTransX, maxTransY;

        if (kScale_Type == fType) {
            maxTransX = size.fWidth  - (1.5f * width);
            maxTransY = size.fHeight - (1.5f * height);
        } else if (kTranslate_Type == fType) {
            maxTransX = SkIntToScalar(size.fWidth  - width);
            maxTransY = SkIntToScalar(size.fHeight - height);
        } else {
            SkASSERT(kRotate_Type == fType);
            // Yes, some rotations will be off the top and left sides
            maxTransX = size.fWidth  - SK_ScalarSqrt2 * height;
            maxTransY = size.fHeight - SK_ScalarSqrt2 * height;
        }

        SkMatrix mat;
        SkRect dst = { 0, 0, SkIntToScalar(width), SkIntToScalar(height) };
        SkRect clearRect = { -1.0f, -1.0f, width+1.0f, height+1.0f };
        SkPoint verts[4] = { // for drawVertices path
            { 0, 0 },
            { 0, SkIntToScalar(height) },
            { SkIntToScalar(width), SkIntToScalar(height) },
            { SkIntToScalar(width), 0 }
        };
        uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };

        SkPaint p;
        p.setColor(0xFF000000);
        p.setFilterLevel(SkPaint::kLow_FilterLevel);

        SkPaint p2;         // for drawVertices path
        p2.setColor(0xFF000000);
        p2.setFilterLevel(SkPaint::kLow_FilterLevel);
        p2.setShader(SkShader::CreateBitmapShader(fAtlas,
                                                  SkShader::kClamp_TileMode,
                                                  SkShader::kClamp_TileMode))->unref();

        for (int i = 0; i < loops; ++i, ++fNumSaved) {
            if (0 == i % kNumBeforeClear) {
                if (kPartial_Clear == fClear) {
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
            if (fAligned) {
                // make the translations integer aligned
                fSaved[fNumSaved][0] = SkScalarFloorToScalar(fSaved[fNumSaved][0]);
                fSaved[fNumSaved][1] = SkScalarFloorToScalar(fSaved[fNumSaved][1]);
            }

            mat.setTranslate(fSaved[fNumSaved][0], fSaved[fNumSaved][1]);

            if (kScale_Type == fType) {
                fSaved[fNumSaved][2] = scaleRand.nextRangeScalar(0.5f, 1.5f);
                mat.preScale(fSaved[fNumSaved][2], fSaved[fNumSaved][2]);
            } else if (kRotate_Type == fType) {
                fSaved[fNumSaved][2] = rotRand.nextRangeScalar(0.0f, 360.0f);
                mat.preRotate(fSaved[fNumSaved][2]);
            }

            canvas->concat(mat);
            if (fUseAtlas) {
                static int curCell = 0;
                SkIRect src = fAtlasRects[curCell % (kNumAtlasedX)][curCell / (kNumAtlasedX)];
                curCell = (curCell + 1) % (kNumAtlasedX*kNumAtlasedY);

                if (fUseDrawVertices) {
                    SkPoint uvs[4] = {
                        { SkIntToScalar(src.fLeft),  SkIntToScalar(src.fBottom) },
                        { SkIntToScalar(src.fLeft),  SkIntToScalar(src.fTop) },
                        { SkIntToScalar(src.fRight), SkIntToScalar(src.fTop) },
                        { SkIntToScalar(src.fRight), SkIntToScalar(src.fBottom) },
                    };
                    canvas->drawVertices(SkCanvas::kTriangles_VertexMode,
                                         4, verts, uvs, NULL, NULL,
                                         indices, 6, p2);
                } else {
                    canvas->drawBitmapRect(fAtlas, &src, dst, &p,
                                           SkCanvas::kBleed_DrawBitmapRectFlag);
                }
            } else {
                canvas->drawBitmapRect(fCheckerboard, NULL, dst, &p);
            }
        }
    }

private:
    static const int kCheckerboardWidth = 64;
    static const int kCheckerboardHeight = 128;

    static const int kAtlasCellWidth = 48;
    static const int kAtlasCellHeight = 36;
    static const int kNumAtlasedX = 5;
    static const int kNumAtlasedY = 5;
    static const int kAtlasSpacer = 2;
    static const int kTotAtlasWidth  = kNumAtlasedX * kAtlasCellWidth +
                                       (kNumAtlasedX+1) * kAtlasSpacer;
    static const int kTotAtlasHeight = kNumAtlasedY * kAtlasCellHeight +
                                       (kNumAtlasedY+1) * kAtlasSpacer;
    static const int kNumBeforeClear = 100;

    Type     fType;
    Clear    fClear;
    bool     fAligned;
    bool     fUseAtlas;
    bool     fUseDrawVertices;
    SkString fName;
    int      fNumSaved; // num draws stored in 'fSaved'
    bool     fInitialized;

    // 0 & 1 are always x & y translate. 2 is either scale or rotate.
    SkScalar fSaved[kNumBeforeClear][3];

    SkBitmap fCheckerboard;
    SkBitmap fAtlas;
    SkIRect  fAtlasRects[kNumAtlasedX][kNumAtlasedY];

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

    // Note: the resulting atlas has transparency
    void makeAtlas() {
        SkRandom rand;

        SkColor colors[kNumAtlasedX][kNumAtlasedY];

        for (int y = 0; y < kNumAtlasedY; ++y) {
            for (int x = 0; x < kNumAtlasedX; ++x) {
                colors[x][y] = rand.nextU() | 0xff000000;
                fAtlasRects[x][y] = SkIRect::MakeXYWH(kAtlasSpacer + x * (kAtlasCellWidth + kAtlasSpacer),
                                                      kAtlasSpacer + y * (kAtlasCellHeight + kAtlasSpacer),
                                                      kAtlasCellWidth,
                                                      kAtlasCellHeight);
            }
        }

        fAtlas.setConfig(SkBitmap::kARGB_8888_Config, kTotAtlasWidth, kTotAtlasHeight);
        fAtlas.allocPixels();
        SkAutoLockPixels lock(fAtlas);

        for (int y = 0; y < kTotAtlasHeight; ++y) {
            int colorY = y / (kAtlasCellHeight + kAtlasSpacer);
            bool inColorY = (y % (kAtlasCellHeight + kAtlasSpacer)) >= kAtlasSpacer;

            SkPMColor* scanline = fAtlas.getAddr32(0, y);

            for (int x = 0; x < kTotAtlasWidth; ++x, ++scanline) {
                int colorX = x / (kAtlasCellWidth + kAtlasSpacer);
                bool inColorX = (x % (kAtlasCellWidth + kAtlasSpacer)) >= kAtlasSpacer;

                if (inColorX && inColorY) {
                    SkASSERT(colorX < kNumAtlasedX && colorY < kNumAtlasedY);
                    *scanline = colors[colorX][colorY];
                } else {
                    *scanline = 0x00000000;
                }
            }
        }
    }

    typedef SkBenchmark INHERITED;
};

// Partial clear
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kScale_Type,
                                            GameBench::kPartial_Clear)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kPartial_Clear)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kPartial_Clear, true)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kRotate_Type,
                                            GameBench::kPartial_Clear)); )

// Full clear
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kScale_Type,
                                            GameBench::kFull_Clear)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kFull_Clear)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kFull_Clear, true)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kRotate_Type,
                                            GameBench::kFull_Clear)); )

// Atlased
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kFull_Clear, false, true)); )
DEF_BENCH( return SkNEW_ARGS(GameBench, (GameBench::kTranslate_Type,
                                            GameBench::kFull_Clear, false, true, true)); )
