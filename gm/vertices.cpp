/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkVerticesPriv.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/utils/SkPatchUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <initializer_list>
#include <utility>

static constexpr SkScalar kShaderSize = 40;
static sk_sp<SkShader> make_shader1(SkScalar shaderScale) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
        SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW,
    };
    const SkPoint pts[] = {{kShaderSize / 4, 0}, {3 * kShaderSize / 4, kShaderSize}};
    const SkMatrix localMatrix = SkMatrix::MakeScale(shaderScale, shaderScale);

    sk_sp<SkShader> grad = SkGradientShader::MakeLinear(pts, colors, nullptr,
                                                        SK_ARRAY_COUNT(colors),
                                                        SkTileMode::kMirror, 0,
                                                        &localMatrix);
    // Throw in a couple of local matrix wrappers for good measure.
    return shaderScale == 1
        ? grad
        : sk_make_sp<SkLocalMatrixShader>(
              sk_make_sp<SkLocalMatrixShader>(std::move(grad), SkMatrix::MakeTrans(-10, 0)),
              SkMatrix::MakeTrans(10, 0));
}

static sk_sp<SkShader> make_shader2() {
    return SkShaders::Color(SK_ColorBLUE);
}

static sk_sp<SkColorFilter> make_color_filter() {
    return SkColorFilters::Blend(0xFFAABBCC, SkBlendMode::kDarken);
}

static constexpr SkScalar kMeshSize = 30;

// start with the center of a 3x3 grid of vertices.
static constexpr uint16_t kMeshFan[] = {
        4,
        0, 1, 2, 5, 8, 7, 6, 3, 0
};

static const int kMeshIndexCnt = (int)SK_ARRAY_COUNT(kMeshFan);
static const int kMeshVertexCnt = 9;

static void fill_mesh(SkPoint pts[kMeshVertexCnt], SkPoint texs[kMeshVertexCnt],
                      SkColor colors[kMeshVertexCnt], SkScalar shaderScale) {
    pts[0].set(0, 0);
    pts[1].set(kMeshSize / 2, 3);
    pts[2].set(kMeshSize, 0);
    pts[3].set(3, kMeshSize / 2);
    pts[4].set(kMeshSize / 2, kMeshSize / 2);
    pts[5].set(kMeshSize - 3, kMeshSize / 2);
    pts[6].set(0, kMeshSize);
    pts[7].set(kMeshSize / 2, kMeshSize - 3);
    pts[8].set(kMeshSize, kMeshSize);

    const auto shaderSize = kShaderSize * shaderScale;
    texs[0].set(0, 0);
    texs[1].set(shaderSize / 2, 0);
    texs[2].set(shaderSize, 0);
    texs[3].set(0, shaderSize / 2);
    texs[4].set(shaderSize / 2, shaderSize / 2);
    texs[5].set(shaderSize, shaderSize / 2);
    texs[6].set(0, shaderSize);
    texs[7].set(shaderSize / 2, shaderSize);
    texs[8].set(shaderSize, shaderSize);

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
    SkScalar                fShaderScale;

public:
    VerticesGM(SkScalar shaderScale) : fShaderScale(shaderScale) {}

protected:

    void onOnceBeforeDraw() override {
        fill_mesh(fPts, fTexs, fColors, fShaderScale);
        fShader1 = make_shader1(fShaderScale);
        fShader2 = make_shader2();
        fColorFilter = make_color_filter();
    }

    SkString onShortName() override {
        SkString name("vertices");
        if (fShaderScale != 1) {
            name.append("_scaled_shader");
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(975, 1175);
    }

    void onDraw(SkCanvas* canvas) override {
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
        for (auto mode : modes) {
            canvas->save();
            for (float alpha : {1.0f, 0.5f}) {
                for (const auto& cf : {sk_sp<SkColorFilter>(nullptr), fColorFilter}) {
                    for (const auto& shader : {fShader1, fShader2}) {
                        static constexpr struct {
                            bool fHasColors;
                            bool fHasTexs;
                        } kAttrs[] = {{true, false}, {false, true}, {true, true}};
                        for (auto attrs : kAttrs) {
                            paint.setShader(shader);
                            paint.setColorFilter(cf);
                            paint.setAlphaf(alpha);

                            const SkColor* colors = attrs.fHasColors ? fColors : nullptr;
                            const SkPoint* texs = attrs.fHasTexs ? fTexs : nullptr;
                            auto v = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
                                                          kMeshVertexCnt, fPts, texs, colors,
                                                          kMeshIndexCnt, kMeshFan);
                            canvas->drawVertices(v, mode, paint);
                            canvas->translate(40, 0);
                            ++x;
                        }
                    }
                }
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM(return new VerticesGM(1);)
DEF_GM(return new VerticesGM(1 / kShaderSize);)

static void draw_batching(SkCanvas* canvas) {
    // Triangle fans can't batch so we convert to regular triangles,
    static constexpr int kNumTris = kMeshIndexCnt - 2;
    SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, kMeshVertexCnt, 3 * kNumTris,
                                SkVertices::kHasColors_BuilderFlag |
                                SkVertices::kHasTexCoords_BuilderFlag);

    SkPoint* pts = builder.positions();
    SkPoint* texs = builder.texCoords();
    SkColor* colors = builder.colors();
    fill_mesh(pts, texs, colors, 1);

    SkTDArray<SkMatrix> matrices;
    matrices.push()->reset();
    matrices.push()->setTranslate(0, 40);
    SkMatrix* m = matrices.push();
    m->setRotate(45, kMeshSize / 2, kMeshSize / 2);
    m->postScale(1.2f, .8f, kMeshSize / 2, kMeshSize / 2);
    m->postTranslate(0, 80);

    auto shader = make_shader1(1);

    uint16_t* indices = builder.indices();
    for (size_t i = 0; i < kNumTris; ++i) {
        indices[3 * i] = kMeshFan[0];
        indices[3 * i + 1] = kMeshFan[i + 1];
        indices[3 * i + 2] = kMeshFan[i + 2];

    }

    canvas->save();
    canvas->translate(10, 10);
    for (bool useShader : {false, true}) {
        for (bool useTex : {false, true}) {
            for (const auto& m : matrices) {
                canvas->save();
                canvas->concat(m);
                SkPaint paint;
                paint.setShader(useShader ? shader : nullptr);

                const SkPoint* t = useTex ? texs : nullptr;
                auto v = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, kMeshVertexCnt,
                                              pts, t, colors, kNumTris * 3, indices);
                canvas->drawVertices(v, SkBlendMode::kModulate, paint);
                canvas->restore();
            }
            canvas->translate(0, 120);
        }
    }
    canvas->restore();
}

// This test exists to exercise batching in the gpu backend.
DEF_SIMPLE_GM(vertices_batching, canvas, 100, 500) {
    draw_batching(canvas);
    canvas->translate(50, 0);
    draw_batching(canvas);
}

using AttrType = SkVertices::Attribute::Type;

DEF_SIMPLE_GM(vertices_data, canvas, 512, 256) {
    for (auto attrType : {AttrType::kFloat4, AttrType::kByte4_unorm}) {
        SkRect r = SkRect::MakeWH(256, 256);
        int vcount = 4;  // just a quad
        int icount = 0;
        SkVertices::Attribute attrs[] = { attrType };
        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, vcount, icount, attrs, 1);

        r.toQuad(builder.positions());

        if (attrType == AttrType::kFloat4) {
            SkV4* col = (SkV4*)builder.customData();
            col[0] = {1, 0, 0, 1};        // red
            col[1] = {0, 1, 0, 1};        // green
            col[2] = {0, 0, 1, 1};        // blue
            col[3] = {0.5, 0.5, 0.5, 1};  // gray
        } else {
            uint32_t* col = (uint32_t*)builder.customData();
            col[0] = 0xFF0000FF;
            col[1] = 0xFF00FF00;
            col[2] = 0xFFFF0000;
            col[3] = 0xFF7F7F7F;
        }

        SkPaint paint;
        const char* gProg = R"(
            varying float4 vtx_color;
            void main(float2 p, inout half4 color) {
                color = half4(vtx_color);
            }
        )";
        auto[effect, errorText] = SkRuntimeEffect::Make(SkString(gProg));
        paint.setShader(effect->makeShader(nullptr, nullptr, 0, nullptr, true));
        canvas->drawVertices(builder.detach(), paint);
        canvas->translate(r.width(), 0);
    }
}

// Test case for skbug.com/10069. We need to draw the vertices twice (with different matrices) to
// trigger the bug.
DEF_SIMPLE_GM(vertices_perspective, canvas, 256, 256) {
    SkPaint paint;
    paint.setShader(ToolUtils::create_checkerboard_shader(SK_ColorBLACK, SK_ColorWHITE, 32));

    SkRect r = SkRect::MakeWH(128, 128);

    SkPoint pos[4];
    r.toQuad(pos);
    auto verts = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4, pos, pos, nullptr);

    SkMatrix persp;
    persp.setPerspY(SK_Scalar1 / 100);

    canvas->save();
    canvas->concat(persp);
    canvas->drawRect(r, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(r.width(), 0);
    canvas->concat(persp);
    canvas->drawRect(r, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(0, r.height());
    canvas->concat(persp);
    canvas->drawVertices(verts, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(r.width(), r.height());
    canvas->concat(persp);
    canvas->drawVertices(verts, paint);
    canvas->restore();
}

DEF_SIMPLE_GM(vertices_data_lerp, canvas, 256, 256) {
    SkPoint pts[12] = {{0, 0},     {85, 0},    {171, 0},  {256, 0}, {256, 85}, {256, 171},
                       {256, 256}, {171, 256}, {85, 256}, {0, 256}, {0, 171},  {0, 85}};

    auto patchVerts = SkPatchUtils::MakeVertices(pts, nullptr, nullptr, 12, 12);
    SkVerticesPriv pv(patchVerts->priv());

    SkVertices::Attribute attrs[1] = { AttrType::kFloat };
    SkVertices::Builder builder(pv.mode(), pv.vertexCount(), pv.indexCount(), attrs, 1);

    memcpy(builder.positions(), pv.positions(), pv.vertexCount() * sizeof(SkPoint));
    memcpy(builder.indices(), pv.indices(), pv.indexCount() * sizeof(uint16_t));

    SkRandom rnd;
    float* lerpData = (float*)builder.customData();
    for (int i = 0; i < pv.vertexCount(); ++i) {
        lerpData[i] = rnd.nextBool() ? 1.0f : 0.0f;
    }

    auto verts = builder.detach();

    SkPaint paint;
    const char* gProg = R"(
        in fragmentProcessor c0;
        in fragmentProcessor c1;
        varying float vtx_lerp;
        void main(float2 p, inout half4 color) {
            half4 col0 = sample(c0, p);
            half4 col1 = sample(c1, p);
            color = mix(col0, col1, half(vtx_lerp));
        }
    )";
    auto [effect, errorText] = SkRuntimeEffect::Make(SkString(gProg));
    SkMatrix scale = SkMatrix::MakeScale(2);
    sk_sp<SkShader> children[] = {
        GetResourceAsImage("images/mandrill_256.png")->makeShader(),
        GetResourceAsImage("images/color_wheel.png")->makeShader(scale),
    };
    paint.setShader(effect->makeShader(nullptr, children, 2, nullptr, false));

    canvas->drawVertices(verts, paint);
}
