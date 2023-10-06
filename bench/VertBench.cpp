/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "src/base/SkRandom.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

// Just want to trigger perspective handling, not dramatically change size
static void tiny_persp_effect(SkCanvas* canvas) {
    SkMatrix m;
    m.reset();
    m[7] = 0.000001f;
    canvas->concat(m);
}

enum VertFlags {
    kColors_VertFlag  = 1 << 0,
    kTexture_VertFlag = 1 << 1,
    kPersp_VertFlag   = 1 << 2,
    kBilerp_VertFlag  = 1 << 3,
};

class VertBench : public Benchmark {
    SkString fName;
    enum {
        W = 64*2,
        H = 48*2,
        ROW = 20,
        COL = 20,
        PTS = (ROW + 1) * (COL + 1),
        IDX = ROW * COL * 6,
    };

    sk_sp<SkShader> fShader;
    SkPoint fPts[PTS], fTex[PTS];
    SkColor fColors[PTS];
    uint16_t fIdx[IDX];
    unsigned fFlags;

    static void load_2_tris(uint16_t idx[], int x, int y, int rb) {
        int n = y * rb + x;
        idx[0] = n; idx[1] = n + 1; idx[2] = rb + n + 1;
        idx[3] = n; idx[4] = rb + n + 1; idx[5] = n + rb;
    }

    void onDelayedSetup() override {
        if (fFlags & kTexture_VertFlag) {
            auto img = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
            if (img) {
                SkFilterMode fm = (fFlags & kBilerp_VertFlag) ? SkFilterMode::kLinear
                                                              : SkFilterMode::kNearest;
                fShader = img->makeShader(SkSamplingOptions(fm));
            }
        }
    }

public:
    VertBench(unsigned flags) : fFlags(flags) {
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

        // We want to store texs in a separate array, so the blitters don't "cheat" and
        // skip the (normal) step of computing the new local-matrix. This is the common case
        // we think in the wild (where the texture coordinates are different from the positions.
        memcpy(fTex, fPts, sizeof(fPts));

        SkRandom rand;
        for (int i = 0; i < PTS; ++i) {
            fColors[i] = rand.nextU() | (0xFF << 24);
        }

        fName.set("verts");
        if (fFlags & kTexture_VertFlag) {
            fName.append("_textures");
        }
        if (fFlags & kColors_VertFlag) {
            fName.append("_colors");
        }
        if (fFlags & kPersp_VertFlag) {
            fName.append("_persp");
        }
        if (fFlags & kBilerp_VertFlag) {
            fName.append("_bilerp");
        }
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setShader(fShader);

        if (fFlags & kPersp_VertFlag) {
            tiny_persp_effect(canvas);
        }

        const SkPoint* texs = (fFlags & kTexture_VertFlag) ? fTex    : nullptr;
        const SkColor* cols = (fFlags & kColors_VertFlag)  ? fColors : nullptr;
        auto verts = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, PTS,
                                          fPts, texs, cols, IDX, fIdx);
        for (int i = 0; i < loops; i++) {
            canvas->drawVertices(verts, SkBlendMode::kModulate, paint);
        }
    }
private:
    using INHERITED = Benchmark;
};
DEF_BENCH(return new VertBench(kTexture_VertFlag | kPersp_VertFlag);)
DEF_BENCH(return new VertBench(kTexture_VertFlag | kPersp_VertFlag | kBilerp_VertFlag);)
DEF_BENCH(return new VertBench(kColors_VertFlag  | kPersp_VertFlag);)
DEF_BENCH(return new VertBench(kTexture_VertFlag);)
DEF_BENCH(return new VertBench(kTexture_VertFlag | kBilerp_VertFlag);)
DEF_BENCH(return new VertBench(kColors_VertFlag);)
DEF_BENCH(return new VertBench(kColors_VertFlag | kTexture_VertFlag);)
DEF_BENCH(return new VertBench(kColors_VertFlag | kTexture_VertFlag | kBilerp_VertFlag);)

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkRSXform.h"
#include "src/base/SkRandom.h"
#include "tools/Resources.h"

enum AtlasFlags {
    kColors_Flag = 1 << 0,
    kRotate_Flag = 1 << 1,
    kPersp_Flag  = 1 << 2,
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
        fName.printf("drawAtlas");
        if (flags & kColors_Flag) {
            fName.append("_colors");
        }
        if (flags & kRotate_Flag) {
            fName.append("_rotated");
        }
        if (flags & kPersp_Flag) {
            fName.append("_persp");
        }
    }
    ~AtlasBench() override {}

protected:
    const char* onGetName() override { return fName.c_str(); }
    void onDelayedSetup() override {
        fAtlas = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
        if (fAtlas) {
            fAtlas = fAtlas->makeRasterImage();
        }

        const SkScalar imageW = fAtlas->width();
        const SkScalar imageH = fAtlas->height();
        SkScalar scos = 1;
        SkScalar ssin = 0;
        if (fFlags & kRotate_Flag) {
            scos = 0.866025403784439f;  // sqrt(3)/2
            ssin = 0.5f;
        }

        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fRects[i] = SkRect::MakeXYWH(rand.nextF() * (imageW - 8),
                                         rand.nextF() * (imageH - 8), 8, 8);
            fColors[i] = rand.nextU() | 0xFF000000;
            fXforms[i] = SkRSXform::Make(scos, ssin, rand.nextF() * W, rand.nextF() * H);
        }
    }
    void onDraw(int loops, SkCanvas* canvas) override {
        const SkRect* cullRect = nullptr;
        const SkPaint* paintPtr = nullptr;
        const SkColor* colors = nullptr;
        if (fFlags & kColors_Flag) {
            colors = fColors;
        }
        if (fFlags & kPersp_Flag) {
            tiny_persp_effect(canvas);
        }
        for (int i = 0; i < loops; i++) {
            canvas->drawAtlas(fAtlas.get(), fXforms, fRects, colors, N, SkBlendMode::kModulate,
                              SkSamplingOptions(), cullRect, paintPtr);
        }
    }
private:
    using INHERITED = Benchmark;
};
//DEF_BENCH(return new AtlasBench(0);)
//DEF_BENCH(return new AtlasBench(kColors_Flag);)
DEF_BENCH(return new AtlasBench(0);)
DEF_BENCH(return new AtlasBench(kRotate_Flag);)
DEF_BENCH(return new AtlasBench(kPersp_Flag);)
DEF_BENCH(return new AtlasBench(kColors_Flag);)
DEF_BENCH(return new AtlasBench(kColors_Flag | kRotate_Flag);)

