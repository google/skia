/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkString.h"
#include "SkVertices.h"

enum VertFlags {
    kColors_VertFlag,
    kTexture_VertFlag,
};

class VertBench : public Benchmark {
    SkString fName;
    enum {
        W = 640,
        H = 480,
        ROW = 20,
        COL = 20,
        PTS = (ROW + 1) * (COL + 1),
        IDX = ROW * COL * 6,
    };

    SkPoint fPts[PTS];
    SkColor fColors[PTS];
    uint16_t fIdx[IDX];

    static void load_2_tris(uint16_t idx[], int x, int y, int rb) {
        int n = y * rb + x;
        idx[0] = n; idx[1] = n + 1; idx[2] = rb + n + 1;
        idx[3] = n; idx[4] = rb + n + 1; idx[5] = n + rb;
    }

public:
    VertBench() {
        const SkScalar dx = SkIntToScalar(W) / COL;
        const SkScalar dy = SkIntToScalar(H) / COL;

        SkPoint* pts = fPts;
        uint16_t* idx = fIdx;

        SkScalar yy = 0;
        for (int y = 0; y <= ROW; y++) {
            SkScalar xx = 0;
            for (int x = 0; x <= COL; ++x) {
                pts->set(xx, yy);
                pts += 1;
                xx += dx;

                if (x < COL && y < ROW) {
                    load_2_tris(idx, x, y, COL + 1);
                    for (int i = 0; i < 6; i++) {
                        SkASSERT(idx[i] < PTS);
                    }
                    idx += 6;
                }
            }
            yy += dy;
        }
        SkASSERT(PTS == pts - fPts);
        SkASSERT(IDX == idx - fIdx);

        SkRandom rand;
        for (int i = 0; i < PTS; ++i) {
            fColors[i] = rand.nextU() | (0xFF << 24);
        }

        fName.set("verts");
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);

        auto verts = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, PTS,
                                          fPts, nullptr, fColors, IDX, fIdx);
        for (int i = 0; i < loops; i++) {
            canvas->drawVertices(verts, SkBlendMode::kModulate, paint);
        }
    }
private:
    typedef Benchmark INHERITED;
};
DEF_BENCH(return new VertBench();)

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Resources.h"
#include "SkRandom.h"
#include "SkRSXform.h"

enum AtlasFlags {
    kColors_Flag = 1 << 0,
    kVerts_Flag  = 1 << 1,
};

class AtlasBench : public Benchmark {
    unsigned fFlags;
    SkString fName;
    enum {
        W = 640,
        H = 480,
        N = 10*1000,
    };

    sk_sp<SkImage>  fAtlas;
    SkRSXform       fXforms[N];
    SkRect          fRects[N];
    SkColor         fColors[N];

public:
    AtlasBench(unsigned flags) : fFlags(flags) {
        fName.printf("drawAtlas_%d", flags);
    }
    ~AtlasBench() override {}

protected:
    const char* onGetName() override { return fName.c_str(); }
    void onDelayedSetup() override {
        fAtlas = GetResourceAsImage("images/mandrill_256.png");
        if (fAtlas) {
            fAtlas = fAtlas->makeRasterImage();
        }

        const SkScalar imageW = fAtlas->width();
        const SkScalar imageH = fAtlas->height();

        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fRects[i] = SkRect::MakeXYWH(rand.nextF() * (imageW - 8),
                                         rand.nextF() * (imageH - 8), 8, 8);
            fColors[i] = rand.nextU();
            fXforms[i] = SkRSXform::Make(1, 0, rand.nextF() * W, rand.nextF() * H);
        }
    }
    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect* cullRect = nullptr;
        const SkPaint* paintPtr = nullptr;
        const SkColor* colors = nullptr;
        const SkImage* atlas = nullptr;
        if (fFlags & kColors_Flag) {
            colors = fColors;
        }
        if (fFlags & kVerts_Flag) {
            atlas = fAtlas.get();
        }
        for (int i = 0; i < loops; i++) {
            canvas->drawAtlas(atlas, fXforms, fRects, colors, N, SkBlendMode::kSrcOver,
                              cullRect, paintPtr);
        }
    }
private:
    typedef Benchmark INHERITED;
};
//DEF_BENCH(return new AtlasBench(0);)
//DEF_BENCH(return new AtlasBench(kColors_Flag);)
DEF_BENCH(return new AtlasBench(kVerts_Flag);)
DEF_BENCH(return new AtlasBench(kVerts_Flag | kColors_Flag);)

