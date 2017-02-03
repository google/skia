/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkRandom.h"

static constexpr SkScalar kShaderSize = 40;
static sk_sp<SkShader> make_shader1() {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
        SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW,
    };
    const SkPoint pts[] = {{kShaderSize / 4, 0}, {3 * kShaderSize / 4, kShaderSize}};

    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkShader::kMirror_TileMode);
}

static sk_sp<SkShader> make_shader2() {
    return SkShader::MakeColorShader(SK_ColorBLUE);
}

static sk_sp<SkColorFilter> make_color_filter() {
    return SkColorFilter::MakeModeFilter(0xFFAABBCC, SkBlendMode::kDarken);
}

static constexpr SkScalar kMeshSize = 30;

// start with the center of a 3x3 grid of vertices.
static constexpr uint16_t kMeshFan[] = {
        4,
        0, 1, 2, 5, 8, 7, 6, 3, 0
};

static const int kMeshVertexCnt = 9;

static void fill_mesh(SkPoint pts[kMeshVertexCnt], SkPoint texs[kMeshVertexCnt],
                      SkColor colors[kMeshVertexCnt]) {
    pts[0].set(0, 0);
    pts[1].set(kMeshSize / 2, 3);
    pts[2].set(kMeshSize, 0);
    pts[3].set(3, kMeshSize / 2);
    pts[4].set(kMeshSize / 2, kMeshSize / 2);
    pts[5].set(kMeshSize - 3, kMeshSize / 2);
    pts[6].set(0, kMeshSize);
    pts[7].set(kMeshSize / 2, kMeshSize - 3);
    pts[8].set(kMeshSize, kMeshSize);

    texs[0].set(0, 0);
    texs[1].set(kShaderSize / 2, 0);
    texs[2].set(kShaderSize, 0);
    texs[3].set(0, kShaderSize / 2);
    texs[4].set(kShaderSize / 2, kShaderSize / 2);
    texs[5].set(kShaderSize, kShaderSize / 2);
    texs[6].set(0, kShaderSize);
    texs[7].set(kShaderSize / 2, kShaderSize);
    texs[8].set(kShaderSize, kShaderSize);

    SkRandom rand;
    for (size_t i = 0; i < kMeshVertexCnt; ++i) {
        colors[i] = rand.nextU() | 0xFF000000;
    }
}

class VerticesGM : public skiagm::GM {
    SkPoint                 fPts[kMeshVertexCnt];
    SkPoint                 fTexs[kMeshVertexCnt];
    SkColor                 fColors[kMeshVertexCnt];
    sk_sp<SkShader>         fShader1;
    sk_sp<SkShader>         fShader2;
    sk_sp<SkColorFilter>    fColorFilter;

public:
    VerticesGM() {}

protected:

    void onOnceBeforeDraw() override {
        fill_mesh(fPts, fTexs, fColors);
        fShader1 = make_shader1();
        fShader2 = make_shader2();
        fColorFilter = make_color_filter();
    }

    SkString onShortName() override {
        SkString name("vertices");
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(600, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        const struct {
            const SkColor*              fColors;
            const SkPoint*              fTexs;
            const sk_sp<SkShader>&      fShader;
            const sk_sp<SkColorFilter>& fColorFilter;
            uint8_t                     fAlpha;
        } rec[] = {
            { fColors,  nullptr, fShader1, nullptr     , 0xFF },
            { nullptr,  fTexs  , fShader1, nullptr     , 0xFF },
            { fColors,  fTexs  , fShader1, nullptr     , 0xFF },
            { fColors,  nullptr, fShader2, nullptr     , 0xFF },
            { nullptr,  fTexs  , fShader2, nullptr     , 0xFF },
            { fColors,  fTexs  , fShader2, nullptr     , 0xFF },
            { fColors,  nullptr, fShader1, fColorFilter, 0xFF },
            { nullptr,  fTexs  , fShader1, fColorFilter, 0xFF },
            { fColors,  fTexs  , fShader1, fColorFilter, 0xFF },
            { fColors,  nullptr, fShader2, fColorFilter, 0xFF },
            { nullptr,  fTexs  , fShader2, fColorFilter, 0xFF },
            { fColors,  fTexs  , fShader2, fColorFilter, 0xFF },
            { fColors,  nullptr, fShader1, nullptr     , 0x80 },
            { nullptr,  fTexs  , fShader1, nullptr     , 0x80 },
            { fColors,  fTexs  , fShader1, nullptr     , 0x80 },
            { fColors,  nullptr, fShader2, nullptr     , 0x80 },
            { nullptr,  fTexs  , fShader2, nullptr     , 0x80 },
            { fColors,  fTexs  , fShader2, nullptr     , 0x80 },
            { fColors,  nullptr, fShader1, fColorFilter, 0x80 },
            { nullptr,  fTexs  , fShader1, fColorFilter, 0x80 },
            { fColors,  fTexs  , fShader1, fColorFilter, 0x80 },
            { fColors,  nullptr, fShader2, fColorFilter, 0x80 },
            { nullptr,  fTexs  , fShader2, fColorFilter, 0x80 },
            { fColors,  fTexs  , fShader2, fColorFilter, 0x80 },
        };

        const SkBlendMode modes[] = {
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
            SkBlendMode::kSrcOut,
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,
            SkBlendMode::kXor,
            SkBlendMode::kPlus,
            SkBlendMode::kModulate,
            SkBlendMode::kScreen,
            SkBlendMode::kOverlay,
            SkBlendMode::kDarken,
            SkBlendMode::kLighten,
            SkBlendMode::kColorDodge,
            SkBlendMode::kColorBurn,
            SkBlendMode::kHardLight,
            SkBlendMode::kSoftLight,
            SkBlendMode::kDifference,
            SkBlendMode::kExclusion,
            SkBlendMode::kMultiply,
            SkBlendMode::kHue,
            SkBlendMode::kSaturation,
            SkBlendMode::kColor,
            SkBlendMode::kLuminosity,
        };

        SkPaint paint;

        canvas->translate(4, 4);
        int x = 0;
        for (size_t j = 0; j < SK_ARRAY_COUNT(modes); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
                paint.setShader(rec[i].fShader);
                paint.setColorFilter(rec[i].fColorFilter);
                paint.setAlpha(rec[i].fAlpha);
                //if (2 == x)
                canvas->drawVertices(SkCanvas::kTriangleFan_VertexMode, kMeshVertexCnt, fPts,
                                     rec[i].fTexs, rec[i].fColors, modes[j], kMeshFan,
                                     SK_ARRAY_COUNT(kMeshFan), paint);
                canvas->translate(40, 0);
                ++x;
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM(return new VerticesGM();)

// This test exists to exercise batching in the gpu backend.
DEF_SIMPLE_GM(vertices_batching, canvas, 50, 500) {
    SkPoint pts[kMeshVertexCnt];
    SkPoint texs[kMeshVertexCnt];
    SkColor colors[kMeshVertexCnt];
    fill_mesh(pts, texs, colors);
    SkTDArray<SkMatrix> matrices;
    matrices.push()->reset();
    matrices.push()->setTranslate(0, 40);
    SkMatrix* m = matrices.push();
    m->setRotate(45, kMeshSize / 2, kMeshSize / 2);
    m->postScale(1.2f, .8f, kMeshSize / 2, kMeshSize / 2);
    m->postTranslate(0, 80);

    auto shader = make_shader1();

    // Triangle fans can't batch so we convert to regular triangles,
    static constexpr int kNumTris = SK_ARRAY_COUNT(kMeshFan) - 2;
    uint16_t indices[3 * kNumTris];
    for (size_t i = 0; i < kNumTris; ++i) {
        indices[3 * i] = kMeshFan[0];
        indices[3 * i + 1] = kMeshFan[i + 1];
        indices[3 * i + 2] = kMeshFan[i + 2];
    }
    canvas->translate(10, 10);
    for (bool useShader : {false, true}) {
        for (bool useTex : {false, true}) {
            for (const auto& m : matrices) {
                canvas->save();
                canvas->concat(m);
                SkPaint paint;
                const SkPoint* t = useTex ? texs : nullptr;
                paint.setShader(useShader ? shader : nullptr);
                canvas->drawVertices(SkCanvas::kTriangles_VertexMode, kMeshVertexCnt, pts, t,
                                     colors, indices, SK_ARRAY_COUNT(indices), paint);
                canvas->restore();
            }
            canvas->translate(0, 120);
        }
    }
}
