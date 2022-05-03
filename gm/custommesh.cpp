/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkCustomMesh.h"
#include "include/core/SkData.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkCanvasPriv.h"

#include <memory>

namespace skiagm {
class CustomMeshGM : public skiagm::GM {
public:
    CustomMeshGM() {}

protected:
    using Attribute = SkCustomMeshSpecification::Attribute;
    using Varying   = SkCustomMeshSpecification::Varying;

    SkISize onISize() override { return {435, 1180}; }

    void onOnceBeforeDraw() override {
        {
            static const Attribute kAttributes[]{
                    {Attribute::Type::kFloat4,       8, SkString{"xuyv"}},
                    {Attribute::Type::kUByte4_unorm, 4, SkString{"brag"}},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kHalf4,  SkString{"color"}},
                    {Varying::Type::kFloat2, SkString{"uv"}   },
            };
            static constexpr char kVS[] = R"(
                    half4 unswizzle_color(half4 color) { return color.garb; }

                    float2 main(in Attributes attributes, out Varyings varyings) {
                        varyings.color = unswizzle_color(attributes.brag);
                        varyings.uv    = attributes.xuyv.yw;
                        return attributes.xuyv.xz;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 main(in Varyings varyings, out float4 color) {
                        color = varyings.color;
                        return varyings.uv;
                    }
            )";
            auto [spec, error] = SkCustomMeshSpecification::Make(
                    SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                    sizeof(ColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithColor = std::move(spec);
        }
        {
            static const Attribute kAttributes[]{
                    {Attribute::Type::kFloat4, 0, SkString{"xuyv"}},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kFloat2, SkString{"vux2"}},
            };
            static constexpr char kVS[] = R"(
                    float2 main(in Attributes a, out Varyings v) {
                        v.vux2 = 2*a.xuyv.wy;
                        return a.xuyv.xz;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 helper(in float2 vux2) { return vux2.yx/2; }
                    float2 main(in Varyings varyings) {
                        return helper(varyings.vux2);
                    }
            )";
            auto [spec, error] = SkCustomMeshSpecification::Make(
                    SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                    sizeof(NoColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithNoColor = std::move(spec);
        }

        static constexpr SkColor kColors[] = {SK_ColorTRANSPARENT, SK_ColorWHITE};
        fShader = SkGradientShader::MakeRadial({10, 10},
                                               3,
                                               kColors,
                                               nullptr,
                                               2,
                                               SkTileMode::kMirror);
        fColorVB = SkCustomMesh::MakeVertexBuffer(
                /*GrDirectContext*=*/nullptr,
                SkData::MakeWithoutCopy(kColorQuad, sizeof(kColorQuad)));

        // Make this one such that the data is offset into the buffer.
        auto data = SkData::MakeUninitialized(sizeof(kNoColorQuad) + kNoColorOffset);
        std::memcpy(SkTAddOffset<void>(data->writable_data(), kNoColorOffset),
                    kNoColorQuad,
                    sizeof(kNoColorQuad));
        fNoColorVB = SkCustomMesh::MakeVertexBuffer(/*GrDirectContext*=*/nullptr, std::move(data));

        fColorIndexedVB = SkCustomMesh::MakeVertexBuffer(
                /*GrDirectContext*=*/nullptr,
                SkData::MakeWithoutCopy(kColorIndexedQuad, sizeof(kColorIndexedQuad)));

        fNoColorIndexedVB = SkCustomMesh::MakeVertexBuffer(
                /*GrDirectContext*=*/nullptr,
                SkData::MakeWithoutCopy(kNoColorIndexedQuad, sizeof(kNoColorIndexedQuad)));

        // Also make index buffer with an offset
        data = SkData::MakeUninitialized(sizeof(kIndices) + kIndexOffset);
        std::memcpy(SkTAddOffset<void>(data->writable_data(), kIndexOffset),
                    kIndices,
                    sizeof(kIndices));
        fIB = SkCustomMesh::MakeIndexBuffer(/*GrDirectContext*=*/nullptr, std::move(data));
    }

    SkString onShortName() override { return SkString("custommesh"); }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        int i = 0;
        for (const sk_sp<SkBlender>& blender : {SkBlender::Mode(SkBlendMode::kDst),
                                                SkBlender::Mode(SkBlendMode::kSrc),
                                                SkBlender::Mode(SkBlendMode::kSaturation)}) {
            canvas->save();
            for (uint8_t alpha  : {0xFF , 0x40})
            for (bool    colors : {false, true})
            for (bool    shader : {false, true}) {
                SkCustomMesh cm;
                // Rather than pile onto the combinatorics we draw every other test case indexed.
                if ((i & 1) == 0) {
                    if (colors) {
                        cm = SkCustomMesh::Make(fSpecWithColor,
                                                SkCustomMesh::Mode::kTriangleStrip,
                                                fColorVB,
                                                /*vertexCount= */4,
                                                /*vertexOffset=*/0,
                                                kRect);
                    } else {
                        cm = SkCustomMesh::Make(fSpecWithNoColor,
                                                SkCustomMesh::Mode::kTriangleStrip,
                                                fNoColorVB,
                                                /*vertexCount=*/4,
                                                kNoColorOffset,
                                                kRect);
                    }
                } else {
                    if (colors) {
                        cm = SkCustomMesh::MakeIndexed(fSpecWithColor,
                                                       SkCustomMesh::Mode::kTriangles,
                                                       fColorIndexedVB,
                                                       /*vertexCount=*/ 6,
                                                       /*vertexOffset=*/0,
                                                       fIB,
                                                       /*indexCount=*/6,
                                                       kIndexOffset,
                                                       kRect);

                    } else {
                        cm = SkCustomMesh::MakeIndexed(fSpecWithNoColor,
                                                       SkCustomMesh::Mode::kTriangles,
                                                       fNoColorIndexedVB,
                                                       /*vertexCount=*/ 6,
                                                       /*vertexOffset=*/0,
                                                       fIB,
                                                       /*indexCount=*/6,
                                                       kIndexOffset,
                                                       kRect);
                    }
                }

                SkPaint paint;
                paint.setColor(SK_ColorGREEN);
                paint.setShader(shader ? fShader : nullptr);
                paint.setAlpha(alpha);

                SkCanvasPriv::DrawCustomMesh(canvas, cm, blender, paint);

                canvas->translate(0, 150);
                ++i;
            }
            canvas->restore();
            canvas->translate(150, 0);
        }
        return DrawResult::kOk;
    }

private:
    struct ColorVertex {
        uint32_t pad;
        uint32_t brag;
        float    xuyv[4];
    };

    struct NoColorVertex {
        float xuyv[4];
    };

    static constexpr auto kRect = SkRect::MakeLTRB(20, 20, 120, 120);
    static constexpr auto kUV   = SkRect::MakeLTRB( 0,  0,  20,  20);

    static constexpr ColorVertex kColorQuad[] {
            {0, 0x00FFFF00, {kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {0, 0x00FFFFFF, {kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {0, 0xFFFF00FF, {kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {0, 0xFFFFFF00, {kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    static constexpr NoColorVertex kNoColorQuad[]{
            {{kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {{kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {{kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {{kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    // The indexed quads draw the same as the non-indexed. They just have unused vertices that the
    // index buffer skips over draw with triangles instead of a triangle strip.
    static constexpr ColorVertex kColorIndexedQuad[] {
            {0, 0x00FFFF00, {kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {0, 0x00000000, {        100.f,        0.f,        100.f,    5.f         }}, // unused
            {0, 0x00FFFFFF, {kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {0, 0x00000000, {        200.f,        10.f,        200.f,   10.f        }}, // unused
            {0, 0xFFFF00FF, {kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {0, 0xFFFFFF00, {kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    static constexpr NoColorVertex kNoColorIndexedQuad[]{
            {{kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {{        100.f,        0.f,        100.f,    5.f         }}, // unused
            {{kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {{        200.f,        10.f,        200.f,   10.f        }}, // unused
            {{kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {{kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    static constexpr uint16_t kIndices[]{0, 2, 4, 2, 5, 4};

    static constexpr size_t kNoColorOffset = sizeof(NoColorVertex);
    static constexpr size_t kIndexOffset   = 6;

    sk_sp<SkShader> fShader;

    sk_sp<SkCustomMeshSpecification> fSpecWithColor;
    sk_sp<SkCustomMeshSpecification> fSpecWithNoColor;

    sk_sp<SkCustomMesh::IndexBuffer> fIB;

    sk_sp<SkCustomMesh::VertexBuffer> fColorVB;
    sk_sp<SkCustomMesh::VertexBuffer> fNoColorVB;
    sk_sp<SkCustomMesh::VertexBuffer> fColorIndexedVB;
    sk_sp<SkCustomMesh::VertexBuffer> fNoColorIndexedVB;
};

DEF_GM( return new CustomMeshGM; )

class CustomMeshColorSpaceGM : public skiagm::GM {
public:
    CustomMeshColorSpaceGM() {}

protected:
    using Attribute = SkCustomMeshSpecification::Attribute;
    using Varying   = SkCustomMeshSpecification::Varying;

    SkISize onISize() override { return {468, 258}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes[]{
                {Attribute::Type::kFloat2, 0, SkString{"pos"}  },
                {Attribute::Type::kFloat4, 8, SkString{"color"}},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kHalf4,  SkString{"color"}},
        };
        static constexpr char kPremulVS[] = R"(
                float2 main(in Attributes attributes, out Varyings varyings) {
                    varyings.color = half4(attributes.color.a*attributes.color.rgb,
                                           attributes.color.a);
                    return attributes.pos;
                }
        )";
        static constexpr char kUnpremulVS[] = R"(
                float2 main(in Attributes attributes, out Varyings varyings) {
                    varyings.color = attributes.color;
                    return attributes.pos;
                }
        )";
        static constexpr char kFS[] = R"(
                void main(in Varyings varyings, out half4 color) {
                    color = varyings.color;
                }
        )";
        for (bool unpremul : {false, true}) {
            auto at = unpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
            auto vs = unpremul ? kUnpremulVS : kPremulVS;
            for (bool spin : {false, true}) {
                auto cs = SkColorSpace::MakeSRGB();
                if (spin) {
                    cs = cs->makeColorSpin();
                }

                auto [spec, error] = SkCustomMeshSpecification::Make(
                        SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                        sizeof(Vertex),
                        SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                        SkString(vs),
                        SkString(kFS),
                        std::move(cs),
                        at);
                if (!spec) {
                    SkDebugf("%s\n", error.c_str());
                }
                fSpecs[SpecIndex(unpremul, spin)] = std::move(spec);
            }
        }
        SkPoint pts[]    = {{kRect.fLeft, 0}, {kRect.centerX(), 0}};
        SkColor colors[] = {SK_ColorWHITE,    SK_ColorTRANSPARENT};
        fShader = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kMirror);

        fVB = SkCustomMesh::MakeVertexBuffer(nullptr,
                                             SkData::MakeWithoutCopy(kQuad, sizeof(kQuad)));
    }

    SkString onShortName() override { return SkString("custommesh_cs"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        // Force an intermediate surface if the canvas is in "legacy" mode
        SkCanvas* c = canvas;
        sk_sp<SkSurface> surface;
        if (!c->imageInfo().colorSpace()) {
            SkImageInfo info = canvas->imageInfo().makeColorSpace(SkColorSpace::MakeSRGB());
            surface = canvas->makeSurface(info);
            if (!surface) {
                // This GM won't work on configs that use a recording canvas.
                return DrawResult::kSkip;
            }
            c = surface->getCanvas();
            c->clear(SK_ColorWHITE);
        }
        for (bool useShader : {false, true})
        for (bool unpremul  : {false, true}) {
            c->save();
            for (bool spin : {false, true}) {
                SkCustomMesh cm = SkCustomMesh::Make(fSpecs[SpecIndex(unpremul, spin)],
                                                     SkCustomMesh::Mode::kTriangleStrip,
                                                     fVB,
                                                     /*vertexCount=*/ 4,
                                                     /*vertexOffset=*/0,
                                                     kRect);

                SkPaint paint;
                paint.setShader(useShader ? fShader : nullptr);
                SkBlendMode mode = useShader ? SkBlendMode::kModulate : SkBlendMode::kDst;
                SkCanvasPriv::DrawCustomMesh(c,
                                             std::move(cm),
                                             SkBlender::Mode(mode),
                                             paint);

                c->translate(0, kRect.height() + 10);
            }
            c->restore();
            c->translate(kRect.width() + 10, 0);
            c->save();
        }
        if (surface) {
            surface->draw(canvas, 0, 0);
        }
        return DrawResult::kOk;
    }

private:
    struct Vertex {
        SkPoint   pos;
        SkColor4f color;
    };

    static int SpecIndex(bool spin, bool unpremul) {
        return static_cast<int>(spin) + 2*static_cast<int>(unpremul);
    }

    static constexpr auto kRect = SkRect::MakeLTRB(20, 20, 120, 120);

    static constexpr Vertex kQuad[] {
            {{kRect.left() , kRect.top()   }, {1, 0, 0, 1}},
            {{kRect.right(), kRect.top()   }, {0, 1, 0, 0}},
            {{kRect.left() , kRect.bottom()}, {1, 1, 0, 0}},
            {{kRect.right(), kRect.bottom()}, {0, 0, 1, 1}},
    };

    sk_sp<SkCustomMesh::VertexBuffer> fVB;

    sk_sp<SkCustomMeshSpecification> fSpecs[4];

    sk_sp<SkShader> fShader;
};

DEF_GM( return new CustomMeshColorSpaceGM; )

}  // namespace skiagm
