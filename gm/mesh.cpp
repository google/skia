/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkMeshGanesh.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/timer/TimeUtils.h"

using namespace skia_private;

namespace skiagm {

class MeshGM : public skiagm::GM {
public:
    MeshGM() {}

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {435, 1180}; }

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

                    Varyings main(const in Attributes attributes) {
                        Varyings varyings;
                        varyings.color    = unswizzle_color(attributes.brag);
                        varyings.uv       = attributes.xuyv.yw;
                        varyings.position = attributes.xuyv.xz;
                        return varyings;
                    }
            )";
            static constexpr char kFS[] = R"(
                    uniform colorFilter filter;

                    float2 main(const in Varyings varyings, out float4 color) {
                        color = filter.eval(varyings.color);
                        return varyings.uv;
                    }
            )";
            auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                           sizeof(ColorVertex),
                                                           kVaryings,
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
                    Varyings main(const in Attributes a) {
                        Varyings v;
                        v.vux2     = 2*a.xuyv.wy;
                        v.position = a.xuyv.xz;
                        return v;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 helper(in float2 vux2) { return vux2.yx/2; }
                    float2 main(const in Varyings varyings) {
                        return helper(varyings.vux2);
                    }
            )";
            auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                           sizeof(NoColorVertex),
                                                           kVaryings,
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
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* string, GraphiteTestContext*) override {
        auto dc = GrAsDirectContext(canvas->recordingContext());
        this->ensureBuffers();
        if (!dc || dc->abandoned()) {
            return DrawResult::kOk;
        }

        fColorVB        = SkMeshes::CopyVertexBuffer(dc, fColorVB);
        fColorIndexedVB = SkMeshes::CopyVertexBuffer(dc, fColorIndexedVB);
        fIB[1]          = SkMeshes::CopyIndexBuffer (dc, fIB[0]);
        if (!fColorVB || !fColorIndexedVB || !fIB[1]) {
            return DrawResult::kFail;
        }
        return DrawResult::kOk;
    }

    void onGpuTeardown() override {
        // Destroy the GPU buffers and recreate on CPU
        fColorVB        = nullptr;
        fColorIndexedVB = nullptr;
        fIB[1]          = nullptr;
        this->ensureBuffers();
    }

    SkString getName() const override { return SkString("custommesh"); }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        SkRuntimeEffect::ChildPtr nullChild[1] = {};
        int i = 0;
        for (const sk_sp<SkBlender>& blender : {SkBlender::Mode(SkBlendMode::kDst),
                                                SkBlender::Mode(SkBlendMode::kSrc),
                                                SkBlender::Mode(SkBlendMode::kSaturation)}) {
            canvas->save();
            for (uint8_t alpha  : {0xFF , 0x40})
            for (bool    colors : {false, true})
            for (bool    shader : {false, true}) {
                SkMesh::Result result;
                // Rather than pile onto the combinatorics we draw every other test case indexed.
                if ((i & 1) == 0) {
                    if (colors) {
                        result = SkMesh::Make(fSpecWithColor,
                                            SkMesh::Mode::kTriangleStrip,
                                            fColorVB,
                                            /*vertexCount=*/4,
                                            /*vertexOffset=*/0,
                                            /*uniforms=*/nullptr,
                                            /*children=*/nullChild,
                                            kRect);
                    } else {
                        result = SkMesh::Make(fSpecWithNoColor,
                                            SkMesh::Mode::kTriangleStrip,
                                            fNoColorVB,
                                            /*vertexCount=*/4,
                                            kNoColorOffset,
                                            /*uniforms=*/nullptr,
                                            /*children=*/{},
                                            kRect);
                    }
                } else {
                    // Alternate between CPU and GPU-backend index buffers.
                    auto ib = (i % 4 == 0) ? fIB[0] : fIB[1];
                    if (colors) {
                        result = SkMesh::MakeIndexed(fSpecWithColor,
                                                     SkMesh::Mode::kTriangles,
                                                     fColorIndexedVB,
                                                     /*vertexCount=*/6,
                                                     kColorIndexedOffset,
                                                     std::move(ib),
                                                     /*indexCount=*/6,
                                                     kIndexOffset,
                                                     /*uniforms=*/nullptr,
                                                     /*children=*/nullChild,
                                                     kRect);
                    } else {
                        result = SkMesh::MakeIndexed(fSpecWithNoColor,
                                                     SkMesh::Mode::kTriangles,
                                                     fNoColorIndexedVB,
                                                     /*vertexCount=*/6,
                                                     /*vertexOffset=*/0,
                                                     std::move(ib),
                                                     /*indexCount=*/6,
                                                     kIndexOffset,
                                                     /*uniforms=*/nullptr,
                                                     /*children=*/{},
                                                     kRect);
                    }
                }
                if (!result.mesh.isValid()) {
                    SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                    return DrawResult::kFail;
                }

                SkPaint paint;
                paint.setColor(SK_ColorGREEN);
                paint.setShader(shader ? fShader : nullptr);
                paint.setAlpha(alpha);

                canvas->drawMesh(result.mesh, blender, paint);

                canvas->translate(0, 150);
                ++i;
            }
            canvas->restore();
            canvas->translate(150, 0);
        }
        return DrawResult::kOk;
    }

private:
    void ensureBuffers() {
        if (!fColorVB) {
            fColorVB = SkMeshes::MakeVertexBuffer(kColorQuad, sizeof(kColorQuad));
        }

        if (!fNoColorVB) {
            // Make this one such that the data is offset into the buffer.
            auto data = SkData::MakeUninitialized(sizeof(kNoColorQuad) + kNoColorOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kNoColorOffset),
                        kNoColorQuad,
                        sizeof(kNoColorQuad));
            fNoColorVB = SkMeshes::MakeVertexBuffer(data->data(), data->size());
        }

        if (!fColorIndexedVB) {
            // This buffer also has an offset.
            auto data = SkData::MakeUninitialized(sizeof(kColorIndexedQuad) + kColorIndexedOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kColorIndexedOffset),
                        kColorIndexedQuad,
                        sizeof(kColorIndexedQuad));
            fColorIndexedVB = SkMeshes::MakeVertexBuffer(data->data(), data->size());
        }

        if (!fNoColorIndexedVB) {
            fNoColorIndexedVB =
                    SkMeshes::MakeVertexBuffer(kNoColorIndexedQuad, sizeof(kNoColorIndexedQuad));
        }

        if (!fIB[0]) {
            // The index buffer has an offset.
            auto data = SkData::MakeUninitialized(sizeof(kIndices) + kIndexOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kIndexOffset),
                        kIndices,
                        sizeof(kIndices));
            fIB[0] = SkMeshes::MakeIndexBuffer(data->data(), data->size());
        }

        if (!fIB[1]) {
            // On CPU we always use the same CPU-backed index buffer.
            fIB[1] = fIB[0];
        }
    }

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

    // For some buffers we add an offset to ensure we're exercising drawing from mid-buffer.
    static constexpr size_t kNoColorOffset      = sizeof(NoColorVertex);
    static constexpr size_t kColorIndexedOffset = 2*sizeof(ColorVertex);
    static constexpr size_t kIndexOffset        = 6;

    sk_sp<SkShader> fShader;

    sk_sp<SkMeshSpecification> fSpecWithColor;
    sk_sp<SkMeshSpecification> fSpecWithNoColor;

    // On GPU the first IB is a CPU buffer and the second is a GPU buffer.
    sk_sp<SkMesh::IndexBuffer> fIB[2];

    sk_sp<SkMesh::VertexBuffer> fColorVB;
    sk_sp<SkMesh::VertexBuffer> fNoColorVB;
    sk_sp<SkMesh::VertexBuffer> fColorIndexedVB;
    sk_sp<SkMesh::VertexBuffer> fNoColorIndexedVB;
};

DEF_GM(return new MeshGM;)

class MeshColorSpaceGM : public skiagm::GM {
public:
    MeshColorSpaceGM() {}

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {468, 258}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes[]{
                {Attribute::Type::kFloat2, 0, SkString{"pos"}  },
                {Attribute::Type::kFloat4, 8, SkString{"color"}},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kHalf4,  SkString{"color"}},
        };
        static constexpr char kPremulVS[] = R"(
                Varyings main(const in Attributes attributes) {
                    Varyings varyings;
                    varyings.color = half4(attributes.color.a*attributes.color.rgb,
                                           attributes.color.a);
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static constexpr char kUnpremulVS[] = R"(
                Varyings main(const in Attributes attributes) {
                    Varyings varyings;
                    varyings.color    = attributes.color;
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static constexpr char kFS[] = R"(
                float2 main(in const Varyings varyings, out half4 color) {
                    color = varyings.color;
                    return varyings.position;
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

                auto [spec, error] = SkMeshSpecification::Make(
                        kAttributes,
                        sizeof(Vertex),
                        kVaryings,
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

        fVB = SkMeshes::MakeVertexBuffer(kQuad, sizeof(kQuad));
    }

    SkString getName() const override { return SkString("custommesh_cs"); }

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
                auto result = SkMesh::Make(fSpecs[SpecIndex(unpremul, spin)],
                                           SkMesh::Mode::kTriangleStrip,
                                           fVB,
                                           /*vertexCount=*/4,
                                           /*vertexOffset=*/0,
                                           /*uniforms=*/nullptr,
                                           /*children=*/{},
                                           kRect);
                if (!result.mesh.isValid()) {
                    SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                    return DrawResult::kFail;
                }

                SkPaint paint;
                paint.setShader(useShader ? fShader : nullptr);
                SkBlendMode mode = useShader ? SkBlendMode::kModulate : SkBlendMode::kDst;
                canvas->drawMesh(result.mesh, SkBlender::Mode(mode), paint);

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

    sk_sp<SkMesh::VertexBuffer> fVB;

    sk_sp<SkMeshSpecification> fSpecs[4];

    sk_sp<SkShader> fShader;
};

// helpers for cases when ctx could be nullptr
static sk_sp<SkMesh::VertexBuffer> make_vertex_buffer(GrDirectContext* ctx,
                                                      const void* data,
                                                      size_t size) {
    if (ctx) {
        return SkMeshes::MakeVertexBuffer(ctx, data, size);
    }
    return SkMeshes::MakeVertexBuffer(data, size);
}

static sk_sp<SkMesh::IndexBuffer> make_index_buffer(GrDirectContext* ctx,
                                                    const void* data,
                                                    size_t size) {
    if (ctx) {
        return SkMeshes::MakeIndexBuffer(ctx, data, size);
    }
    return SkMeshes::MakeIndexBuffer(data, size);
}

DEF_GM(return new MeshColorSpaceGM;)

class MeshUniformsGM : public skiagm::GM {
public:
    MeshUniformsGM() { this->onAnimate(0); }

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {140, 250}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes[]{
                {Attribute::Type::kFloat2, 0, SkString{"pos"}  },
                {Attribute::Type::kFloat2, 8, SkString{"coords"}},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kFloat2, SkString{"coords"}},
        };
        // To exercise shared VS/FS uniforms we have a matrix that is applied twice, once in each
        // stage.
        static constexpr char kVS[] = R"(
                uniform float t[2];
                uniform half3x3 m;
                Varyings main(in const Attributes attributes) {
                    Varyings varyings;
                    varyings.coords   = (m*float3(attributes.coords + float2(t[0], t[1]), 1)).xy;
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static constexpr char kFS[] = R"(
                uniform half3x3 m;
                layout(color) uniform half4 color;
                float2 main(const Varyings varyings, out half4 c) {
                    c = color;
                    return (m*float3(varyings.coords, 1)).xy;
                }
        )";
        auto [spec, error] =
                SkMeshSpecification::Make(kAttributes,
                                          sizeof(Vertex),
                                          kVaryings,
                                          SkString(kVS),
                                          SkString(kFS),
                                          SkColorSpace::MakeSRGB(),
                                          kPremul_SkAlphaType);
        if (!spec) {
            SkDebugf("%s\n", error.c_str());
        }
        fSpec = std::move(spec);

        SkColor colors[] = {SK_ColorWHITE, SK_ColorBLACK};
        fShader = SkGradientShader::MakeRadial(kGradCenter,
                                               .4f,
                                               colors,
                                               nullptr,
                                               2,
                                               SkTileMode::kMirror);

        fVB = SkMeshes::MakeVertexBuffer(kQuad, sizeof(kQuad));
    }

    SkString getName() const override { return SkString("custommesh_uniforms"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        SkMatrix matrices[] {
                SkMatrix::MakeAll(-1,  0, 0, // self inverse so no effect.
                                   0, -1, 0,
                                   0,  0, 1),
                SkMatrix::RotateDeg(fDegrees/2.f, {0.5f, 0.5f}),
        };
        for (const auto& m : matrices) {
            auto unis = SkData::MakeUninitialized(fSpec->uniformSize());

            SkPoint trans = -kCoordTrans;
            static_assert(sizeof(SkPoint) == 2*sizeof(float));

            const SkMeshSpecification::Uniform* u = fSpec->findUniform("t");
            SkASSERT(u);
            std::memcpy(SkTAddOffset<void>(unis->writable_data(), u->offset),
                        (void*)&trans,
                        2*sizeof(float));

            u = fSpec->findUniform("m");
            SkASSERT(u);
            for (size_t offset = u->offset, col = 0; col < 3; ++col) {
                for (size_t row = 0; row < 3; ++row, offset += sizeof(float)) {
                    *SkTAddOffset<float>(unis->writable_data(), offset) = m.rc(row, col);
                }
            }

            u = fSpec->findUniform("color");
            SkASSERT(u);
            std::memcpy(SkTAddOffset<void>(unis->writable_data(), u->offset),
                        fColor.vec(),
                        4*sizeof(float));

            auto result = SkMesh::Make(fSpec,
                                       SkMesh::Mode::kTriangleStrip,
                                       fVB,
                                       /*vertexCount=*/4,
                                       /*vertexOffset=*/0,
                                       /*uniforms=*/std::move(unis),
                                       /*children=*/{},
                                       kRect);

            if (!result.mesh.isValid()) {
                SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                return DrawResult::kFail;
            }

            SkPaint paint;
            paint.setShader(fShader);
            canvas->drawMesh(result.mesh, SkBlender::Mode(SkBlendMode::kModulate), paint);

            canvas->translate(0, kRect.height() + 10);
        }
        return DrawResult::kOk;
    }

    bool onAnimate(double nanos) override {
        fDegrees = TimeUtils::NanosToSeconds(nanos) * 360.f/10.f + 45.f;
        // prime number periods, like locusts.
        fColor.fR = TimeUtils::SineWave(nanos, 13.f, 0.f, 0.f, 1.f);
        fColor.fG = TimeUtils::SineWave(nanos, 23.f, 5.f, 0.f, 1.f);
        fColor.fB = TimeUtils::SineWave(nanos, 11.f, 0.f, 0.f, 1.f);
        fColor.fA = 1.f;
        return true;
    }

private:
    struct Vertex {
        SkPoint pos;
        SkPoint tex;
    };

    static constexpr auto kRect = SkRect::MakeLTRB(20, 20, 120, 120);

    // Our logical tex coords are [0..1] but we insert an arbitrary translation that gets undone
    // with a uniform.
    static constexpr SkPoint kCoordTrans = {75, -37};
    static constexpr auto    kCoordRect  = SkRect::MakeXYWH(kCoordTrans.x(), kCoordTrans.y(), 1, 1);

    static constexpr SkPoint kGradCenter = {0.3f, 0.2f};

    static constexpr Vertex kQuad[] {
            {{kRect.left() , kRect.top()   }, {kCoordRect.left() , kCoordRect.top()}   },
            {{kRect.right(), kRect.top()   }, {kCoordRect.right(), kCoordRect.top()}   },
            {{kRect.left() , kRect.bottom()}, {kCoordRect.left() , kCoordRect.bottom()}},
            {{kRect.right(), kRect.bottom()}, {kCoordRect.right(), kCoordRect.bottom()}},
    };

    float fDegrees;

    SkColor4f fColor;

    sk_sp<SkMesh::VertexBuffer> fVB;

    sk_sp<SkMeshSpecification> fSpec;

    sk_sp<SkShader> fShader;
};

DEF_GM(return new MeshUniformsGM())

class MeshUpdateGM : public skiagm::GM {
public:
    MeshUpdateGM() = default;

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying = SkMeshSpecification::Varying;

    SkISize getISize() override { return {270, 490}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes[]{
                {Attribute::Type::kFloat2, 0, SkString{"pos"}},
                {Attribute::Type::kFloat2, 8, SkString{"coords"}},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kFloat2, SkString{"coords"}},
        };
        static constexpr char kVS[] = R"(
                Varyings main(const in Attributes attributes) {
                    Varyings varyings;
                    varyings.coords   = attributes.coords;
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static constexpr char kFS[] = R"(
                float2 main(const Varyings varyings) { return varyings.coords; }
        )";
        auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                       sizeof(Vertex),
                                                       kVaryings,
                                                       SkString(kVS),
                                                       SkString(kFS),
                                                       SkColorSpace::MakeSRGB(),
                                                       kPremul_SkAlphaType);
        if (!spec) {
            SkDebugf("%s\n", error.c_str());
        }
        fSpec = std::move(spec);

        uint32_t colors[] = {SK_ColorYELLOW, SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorWHITE};
        SkPixmap pixmap(SkImageInfo::Make({2, 2}, kBGRA_8888_SkColorType, kPremul_SkAlphaType),
                        colors,
                        /*rowBytes=*/8);
        fShader = SkImages::RasterFromPixmapCopy(pixmap)->makeShader(
                SkTileMode::kClamp, SkTileMode::kClamp, SkFilterMode::kLinear);
    }

    SkString getName() const override { return SkString("mesh_updates"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        canvas->clear(SK_ColorBLACK);

        GrRecordingContext* rc = canvas->recordingContext();
        GrDirectContext* dc = GrAsDirectContext(rc);
        if (rc && !dc) {
            // On GPU this relies on using the DC to update the GPU backed vertex/index buffers.
            return DrawResult::kSkip;
        }

        if (dc && dc->abandoned()) {
            return DrawResult::kSkip;
        }

        SkPaint paint;
        paint.setShader(fShader);

        SkRect r = SkRect::MakeXYWH(10.f, 10.f, 50.f, 50.f);

        // We test updating CPU and GPU buffers.
        for (bool gpuBuffer : {false, true}) {
            auto ctx = gpuBuffer ? dc : nullptr;

            // How many rects worth of storage is in the vertex buffer?
            static constexpr int kVBRects = 2;

            // How many times do we update the vertex buffer? Wraps to start of buffer if
            // > kVBRects.
            static constexpr int kUpdatesRects = 3;

            auto vb = make_vertex_buffer(ctx, /*data=*/nullptr, kVBRects * 6 * sizeof(Vertex));
            SkASSERT(vb);

            SkRect bounds;
            for (int i = 0; i < kUpdatesRects; ++i) {
                auto p = r.makeOffset(100.f*i, 0.f);
                if (i) {
                    bounds.join(p);
                } else {
                    bounds = p;
                }

                SkPoint t[4];
                SkRect::MakeWH(2.f, 2.f).toQuad(t);
                SkMatrix::RotateDeg(90.f*i, {1.f, 1.f}).mapPoints(t, std::size(t));

                Vertex vertices[6];
                vertices[0] = {{p.left(), p.top()}, t[0]};
                vertices[1] = {{p.left(), p.bottom()}, t[3]};
                vertices[2] = {{p.right(), p.top()}, t[1]};
                vertices[3] = vertices[2];
                vertices[4] = vertices[1];
                vertices[5] = {{p.right(), p.bottom()}, t[2]};

                size_t offset = 6*(i % kVBRects)*sizeof(Vertex);
                SkAssertResult(vb->update(ctx, vertices, offset, 6*sizeof(Vertex)));
                // Make there aren't accidentally deferred reads of the client data.
                std::memset(vertices, 0, sizeof(vertices));

                int rectCount = std::min(i + 1, kVBRects);
                auto result = SkMesh::Make(fSpec,
                                           SkMesh::Mode::kTriangles,
                                           vb,
                                           /*vertexCount=*/6 * rectCount,
                                           /*vertexOffset=*/0,
                                           /*uniforms=*/nullptr,
                                           /*children=*/{},
                                           bounds);

                if (!result.mesh.isValid()) {
                    SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                    return DrawResult::kFail;
                }

                canvas->drawMesh(result.mesh, SkBlender::Mode(SkBlendMode::kDst), paint);

                canvas->translate(0, r.height() + 10);
            }

            // Now test updating an IB.

            // How many rects worth of storage is in the index buffer?
            static constexpr int kIBRects = 2;

            // How many times do we update the index buffer? Wraps to start of buffer if > kIBRects.
            static constexpr int kNumIBUpdates = 3;

            // Make the vertex buffer large enough to hold all the rects and populate.
            vb = make_vertex_buffer(ctx, /*data=*/nullptr, kNumIBUpdates * 4 * sizeof(Vertex));
            SkASSERT(vb);
            for (int i = 0; i < kNumIBUpdates; ++i) {
                SkPoint p[4];
                auto rect = r.makeOffset(100*i, 0);
                rect.toQuad(p);
                if (i) {
                    bounds.join(rect);
                } else {
                    bounds = rect;
                }

                SkPoint t[4];
                SkRect::MakeWH(2.f, 2.f).toQuad(t);
                SkMatrix::RotateDeg(90.f*i, {1.f, 1.f}).mapPoints(t, std::size(t));
                Vertex vertices[4]{{p[0], t[0]}, {p[1], t[1]}, {p[2], t[2]}, {p[3], t[3]}};
                SkAssertResult(
                        vb->update(ctx, vertices, i*4*sizeof(Vertex), 4*sizeof(Vertex)));
            }

            auto ib = make_index_buffer(
                    ctx, /*data=*/nullptr, kIBRects * 6 * sizeof(uint16_t));
            SkASSERT(ib);
            for (int i = 0; i < kNumIBUpdates; ++i) {
                uint16_t indices[6] = {SkToU16(0 + 4*i),
                                       SkToU16(3 + 4*i),
                                       SkToU16(1 + 4*i),
                                       SkToU16(1 + 4*i),
                                       SkToU16(3 + 4*i),
                                       SkToU16(2 + 4*i)};
                size_t offset = 6*(i % kIBRects)*sizeof(uint16_t);
                SkAssertResult(ib->update(ctx, indices, offset, 6*sizeof(uint16_t)));
                std::memset(indices, 0, 6*sizeof(uint16_t));

                auto result = SkMesh::MakeIndexed(fSpec,
                                                  SkMesh::Mode::kTriangles,
                                                  vb,
                                                  /*vertexCount=*/4 * kNumIBUpdates,
                                                  /*vertexOffset=*/0,
                                                  ib,
                                                  /*indexCount=*/6,
                                                  /*indexOffset=*/offset,
                                                  /*uniforms=*/nullptr,
                                                  /*children=*/{},
                                                  bounds);

                if (!result.mesh.isValid()) {
                    SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                    return DrawResult::kFail;
                }

                canvas->drawMesh(result.mesh, SkBlender::Mode(SkBlendMode::kDst), paint);
            }
            canvas->translate(0, r.height() + 10);
        }

        return DrawResult::kOk;
    }

private:
    struct Vertex {
        SkPoint pos;
        SkPoint tex;
    };

    sk_sp<SkMeshSpecification> fSpec;

    sk_sp<SkShader> fShader;
};

DEF_GM(return new MeshUpdateGM())

class MeshZeroInitGM : public skiagm::GM {
public:
    MeshZeroInitGM() = default;

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {90, 30}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes1[]{
                {Attribute::Type::kUByte4_unorm, 0, SkString{"color"}},
                {Attribute::Type::kFloat2,       4, SkString{"pos"  }},
        };
        static const Attribute kAttributes2[]{
                {Attribute::Type::kFloat2,       0, SkString{"pos"  }},
                {Attribute::Type::kUByte4_unorm, 8, SkString{"color"}},
        };
        static const Varying kVaryings[]{{Varying::Type::kHalf4, SkString{"color"}}};
        static constexpr char kVS[] = R"(
                Varyings main(const in Attributes attributes) {
                    Varyings varyings;
                    varyings.color    = attributes.color;
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static constexpr char kFS[] = R"(
                float2 main(const Varyings varyings, out half4 color) {
                    color = varyings.color;
                    return varyings.position;
                }
        )";
        auto result = SkMeshSpecification::Make(kAttributes1,
                                                /*vertexStride==*/12,
                                                kVaryings,
                                                SkString(kVS),
                                                SkString(kFS),
                                                SkColorSpace::MakeSRGB(),
                                                kPremul_SkAlphaType);
        if (!result.specification) {
            SkDebugf("%s\n", result.error.c_str());
        }
        fSpec[0] = std::move(result.specification);

        result = SkMeshSpecification::Make(kAttributes1,
                                           /*vertexStride=*/12,
                                           kVaryings,
                                           SkString(kVS),
                                           SkString(kFS),
                                           SkColorSpace::MakeSRGB(),
                                           kPremul_SkAlphaType);
        if (!result.specification) {
            SkDebugf("%s\n", result.error.c_str());
        }
        fSpec[1] = std::move(result.specification);
    }

    SkString getName() const override { return SkString("mesh_zero_init"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        GrRecordingContext* rc = canvas->recordingContext();
        GrDirectContext* dc = GrAsDirectContext(rc);
        if (rc && !dc) {
            // On GPU this relies on using the DC to update the GPU backed vertex/index buffers.
            return DrawResult::kSkip;
        }

        if (dc && dc->abandoned()) {
            return DrawResult::kSkip;
        }

        static constexpr SkPoint kTri[]{{10, 10}, {20, 10}, {10, 20}};
        // The zero will come from the uninit part of the buffer.
        static constexpr uint16_t kTiIndices[]{1, 2};

        // We test updating CPU and GPU buffers.
        for (bool gpuBuffer : {false, true}) {
            auto ctx = gpuBuffer ? dc : nullptr;
            for (int i = 0; i < 2; ++i) {
                const auto& spec = fSpec[i];

                size_t posOffset = spec->findAttribute("pos")->offset;
                auto vb = make_vertex_buffer(ctx, nullptr, spec->stride() * std::size(kTri));
                SkASSERT(vb);
                for (size_t j = 0; j < std::size(kTri); ++j) {
                    SkAssertResult(vb->update(ctx,
                                              &kTri[j],
                                              spec->stride()*j + posOffset,
                                              sizeof(kTri[j])));
                }

                // The first time we make the indices be 0,1,2 using the zero'ed buffer for the
                // first. However, because uploads must be 4 byte aligned it's actually 0,0,1,2.
                // The second time we upload 1,2 to beginning of the buffer to form 1,2,0.
                size_t indexUploadOffset = i == 0 ? 4 : 0;
                size_t indexMeshOffset   = i == 0 ? 2 : 0;

                auto ib = make_index_buffer(ctx, nullptr, sizeof(uint16_t) * 4);
                SkASSERT(ib);
                SkAssertResult(ib->update(ctx, kTiIndices, indexUploadOffset, sizeof(kTiIndices)));

                SkRect bounds;
                bounds.setBounds(kTri, std::size(kTri));
                auto result = SkMesh::MakeIndexed(spec,
                                                  SkMesh::Mode::kTriangles,
                                                  std::move(vb),
                                                  /*vertexCount=*/std::size(kTri),
                                                  /*vertexOffset=*/0,
                                                  std::move(ib),
                                                  /*indexCount=*/std::size(kTiIndices) + 1,
                                                  indexMeshOffset,
                                                  /*uniforms=*/nullptr,
                                                  /*children=*/{},
                                                  bounds);
                if (!result.mesh.isValid()) {
                    SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
                    return DrawResult::kFail;
                }

                SkPaint paint;
                // The color will be transparent black. Set the blender to kDstOver so when combined
                // with the paint's opaque black we get opaque black.
                canvas->drawMesh(result.mesh, SkBlender::Mode(SkBlendMode::kDstOver), paint);
                canvas->translate(bounds.width() + 10, 0);
                if (ctx) {
                    // Free up the buffers for recycling in the cache. This helps test that
                    // a recycled buffer gets zero'ed.
                    result.mesh = {};
                    SkASSERT(!ib);  // NOLINT - bugprone-use-after-move. We're asserting it's moved.
                    SkASSERT(!vb);  // NOLINT
                    ctx->flushAndSubmit(GrSyncCpu::kYes);
                }
            }
        }

        return DrawResult::kOk;
    }

private:
    sk_sp<SkMeshSpecification> fSpec[2];
};

DEF_GM(return new MeshZeroInitGM())

// We have a special GM for testing SkMesh through SkPicture because all of SkPicture GM testing
// uses the CPU backend and SkMesh only works on GPU.
class PictureMesh : public skiagm::GM {
public:
    PictureMesh() = default;

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {390, 90}; }

    void onOnceBeforeDraw() override {
        static const Attribute kAttributes[]{
                {Attribute::Type::kFloat2, 0, SkString{"pos"}},
        };
        static const Varying kVaryings[]{
                {Varying::Type::kFloat2, SkString{"coords"}},
        };
        static constexpr char kVS[] = R"(
                Varyings main(in const Attributes attributes) {
                    Varyings varyings;
                    varyings.position = attributes.pos;
                    return varyings;
                }
        )";
        static const SkString kFS = SkStringPrintf(R"(
                uniform float2 offset;
                float2 main(const Varyings varyings, out float4 color) {
                    float2 tl = float2(%f, %f);
                    float2 wh = float2(%f, %f);
                    float2 c = tl + wh/2;
                    float  r = length(wh)/4;
                    color.rba = float3(1);
                    color.g = min(1, length(varyings.position - c + offset) / r);
                    return varyings.position;
                }
        )", kRect.x(), kRect.y(), kRect.width(), kRect.height());
        auto [spec, error] =
                SkMeshSpecification::Make(kAttributes,
                                          sizeof(Vertex),
                                          kVaryings,
                                          SkString(kVS),
                                          kFS,
                                          SkColorSpace::MakeSRGB()->makeColorSpin(),
                                          kPremul_SkAlphaType);
        if (!spec) {
            SkDebugf("%s\n", error.c_str());
        }
        fSpec = std::move(spec);

        fVB = SkMeshes::MakeVertexBuffer(kQuad, sizeof(kQuad));
        fIB = SkMeshes::MakeIndexBuffer(kIndices, sizeof(kIndices));

        SkRandom random;
        SkColor4f colors[6];
        for (size_t i = 0; i < std::size(colors) - 1; ++i) {
            colors[i] = {random.nextF(), random.nextF(), random.nextF(), 1.f};
        }
        colors[std::size(colors) - 1] = colors[0];
        SkPaint paint;
        SkGradientShader::Interpolation interpolation;
        interpolation.fColorSpace = SkGradientShader::Interpolation::ColorSpace::kHSL;
        fShader = SkGradientShader::MakeSweep(kRect.centerX(), kRect.centerY(),
                                              colors,
                                              SkColorSpace::MakeSRGB(),
                                              nullptr,
                                              std::size(colors),
                                              SkTileMode::kRepeat,
                                              0,
                                              360.f,
                                              interpolation,
                                              /*localMatrix=*/nullptr);
    }

    SkString getName() const override { return SkString("picture_mesh"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        SkPaint paint;
        paint.setShader(fShader);

        auto dc = GrAsDirectContext(canvas->recordingContext());
        for (bool picture : {false, true}) {
            canvas->save();
            for (bool gpu : {false, true}) {
                auto vb = gpu ? SkMeshes::CopyVertexBuffer(dc, fVB) : fVB;
                auto ib = gpu ? SkMeshes::CopyIndexBuffer (dc, fIB) : fIB;

                float offset[2] = {8, 8};
                for (size_t i = 0; i < 4; ++i) {
                    auto uniforms = SkData::MakeWithCopy(&offset, sizeof(offset));
                    SkMesh::Result r;
                    switch (i) {
                        case 0:
                            r = SkMesh::Make(fSpec,
                                             SkMesh::Mode::kTriangles,
                                             fVB,
                                             6,
                                             1 * sizeof(Vertex),
                                             std::move(uniforms),
                                             /*children=*/{},
                                             kRect);
                            break;
                        case 1:
                            r = SkMesh::Make(fSpec,
                                             SkMesh::Mode::kTriangleStrip,
                                             fVB,
                                             4,
                                             1 * sizeof(Vertex),
                                             std::move(uniforms),
                                             /*children=*/{},
                                             kRect);
                            break;
                        case 2:
                            r = SkMesh::MakeIndexed(fSpec,
                                                    SkMesh::Mode::kTriangles,
                                                    fVB,
                                                    std::size(kQuad),
                                                    0,
                                                    fIB,
                                                    6,
                                                    2 * (sizeof(uint16_t)),
                                                    std::move(uniforms),
                                                    /*children=*/{},
                                                    kRect);
                            break;
                        case 3:
                            r = SkMesh::MakeIndexed(fSpec,
                                                    SkMesh::Mode::kTriangleStrip,
                                                    fVB,
                                                    std::size(kQuad),
                                                    0,
                                                    fIB,
                                                    6,
                                                    2 * sizeof(uint16_t),
                                                    std::move(uniforms),
                                                    /*children=*/{},
                                                    kRect);
                            break;
                    }

                    if (!r.mesh.isValid()) {
                        *error = r.error;
                        return DrawResult::kFail;
                    }

                    auto draw = [&](SkCanvas* c) {
                        c->drawMesh(r.mesh, SkBlender::Mode(SkBlendMode::kDifference), paint);
                    };
                    if (picture) {
                        SkPictureRecorder recorder;
                        draw(recorder.beginRecording(SkRect::Make(this->getISize()),
                                                     /*bbhFactory=*/nullptr));
                        canvas->drawPicture(recorder.finishRecordingAsPicture());
                    } else {
                        draw(canvas);
                    }
                    offset[i%2] *= -1;
                    canvas->translate(kRect.width() + 10, 0);
                }
            }
            canvas->restore();
            canvas->translate(0, kRect.height() + 10);
        }
        return DrawResult::kOk;
    }

private:
    struct Vertex {
        SkPoint pos;
    };

    static constexpr auto kRect = SkRect::MakeWH(40, 40);

    static constexpr Vertex kQuad[] {
            {1000, 1000},  // skip
            {{kRect.left() , kRect.top()   }},
            {{kRect.right(), kRect.top()   }},
            {{kRect.left() , kRect.bottom()}},
            {{kRect.right(), kRect.bottom()}},
            {{kRect.left() , kRect.bottom()}},
            {{kRect.right(), kRect.top()   }},
    };

    static constexpr uint16_t kIndices[] = {1000, 2000, 1, 2, 3, 4, 5, 6};

    sk_sp<SkMesh::VertexBuffer> fVB;

    sk_sp<SkMesh::IndexBuffer> fIB;

    sk_sp<SkMeshSpecification> fSpec;

    sk_sp<SkShader> fShader;
};

DEF_GM(return new PictureMesh())

class MeshWithShadersGM : public skiagm::GM {
public:
    enum class Type {
        kMeshWithImage,
        kMeshWithPaintColor,
        kMeshWithPaintImage,
        kMeshWithEffects,
    };

    MeshWithShadersGM(Type type) : fType(type) {
        // Create a grid of evenly spaced points for our mesh
        this->onAnimate(0.0);

        // Create an index buffer of triangles over our point mesh.
        for (int y = 0; y < kMeshSize - 1; ++y) {
            for (int x = 0; x < kMeshSize - 1; ++x) {
                SkASSERT(((y + 1) * kMeshSize + x + 1) < fVerts.size());

                uint16_t TL =  y      * kMeshSize + x;
                uint16_t TR =  y      * kMeshSize + x + 1;
                uint16_t BL = (y + 1) * kMeshSize + x;
                uint16_t BR = (y + 1) * kMeshSize + x + 1;

                fIndices.push_back(TL);
                fIndices.push_back(TR);
                fIndices.push_back(BL);

                fIndices.push_back(BR);
                fIndices.push_back(BL);
                fIndices.push_back(TR);
            }
        }
    }

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize getISize() override { return {320, 320}; }

    void onOnceBeforeDraw() override {
        {
            static const Attribute kAttributes[] = {
                    {Attribute::Type::kFloat2, 0, SkString{"position"}},
                    {Attribute::Type::kFloat2, 8, SkString{"uv"}},
            };
            static const Varying kVaryings[] = {
                    {Varying::Type::kFloat2, SkString{"uv"}},
            };
            static constexpr char kVS[] = R"(
                    Varyings main(const in Attributes attributes) {
                        Varyings varyings;
                        varyings.uv       = attributes.uv;
                        varyings.position = attributes.position;
                        return varyings;
                    }
            )";
            static constexpr char kFS[] = R"(
                    uniform shader myShader1;
                    uniform shader myShader2;
                    uniform colorFilter myColorFilter;
                    uniform blender myBlend;

                    float2 main(const in Varyings varyings, out half4 color) {
                        half4 color1 = myShader1.eval(varyings.uv);
                        half4 color2 = myShader2.eval(varyings.uv);

                        // Apply a inverse color filter to the first image.
                        color1 = myColorFilter.eval(color1);

                        // Fade in the second image horizontally, leveraging the UVs.
                        color2 *= varyings.uv.x / 128.0;

                        // Combine the two images by using a blender (set to dst-over).
                        color = myBlend.eval(color1, color2);

                        return varyings.uv;
                    }
            )";
            auto [spec, error] = SkMeshSpecification::Make(kAttributes,
                                                           sizeof(Vertex),
                                                           kVaryings,
                                                           SkString(kVS),
                                                           SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpec = std::move(spec);
        }


        switch (fType) {
            case Type::kMeshWithImage: {
                fShader1 = ToolUtils::GetResourceAsImage("images/mandrill_128.png")
                                   ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                fShader2 = nullptr;
                fColorFilter = nullptr;
                fBlender = nullptr;
                fPaintShader = nullptr;
                break;
            }
            case Type::kMeshWithEffects: {
                uint8_t inverseTable[256];
                for (int index = 0; index < 256; ++index) {
                    inverseTable[index] = 255 - index;
                }

                fShader1 = ToolUtils::GetResourceAsImage("images/mandrill_128.png")
                                   ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                fShader2 = ToolUtils::GetResourceAsImage("images/color_wheel.png")
                                   ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                fColorFilter = SkColorFilters::TableARGB(/*tableA=*/nullptr,
                                                         inverseTable,
                                                         inverseTable,
                                                         inverseTable);
                fBlender = SkBlender::Mode(SkBlendMode::kDstOver);
                fPaintShader = nullptr;
                break;
            }
            case Type::kMeshWithPaintColor: {
                fShader1 = nullptr;
                fShader2 = ToolUtils::GetResourceAsImage("images/mandrill_128.png")
                                   ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                fColorFilter = nullptr;
                fBlender = SkBlender::Mode(SkBlendMode::kDst);
                fPaintShader = SkShaders::Color(SK_ColorGREEN);
                break;
            }
            case Type::kMeshWithPaintImage: {
                fShader1 = ToolUtils::GetResourceAsImage("images/color_wheel.png")
                                   ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                fShader2 = nullptr;
                fColorFilter = nullptr;
                fBlender = nullptr;
                fPaintShader = ToolUtils::GetResourceAsImage("images/mandrill_128.png")
                                       ->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
                break;
            }
            default:
                SkUNREACHABLE;
        }
    }

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* string, GraphiteTestContext*) override {
        auto dc = GrAsDirectContext(canvas->recordingContext());
        this->ensureBuffers();
        if (!dc || dc->abandoned()) {
            return DrawResult::kOk;
        }

        fVB = SkMeshes::CopyVertexBuffer(dc, fVB);
        fIB = SkMeshes::CopyIndexBuffer (dc, fIB);
        return (!fVB || !fIB) ? DrawResult::kFail
                              : DrawResult::kOk;
    }

    void onGpuTeardown() override {
        // Destroy the GPU buffers and recreate on CPU
        fVB = nullptr;
        fIB = nullptr;
        this->ensureBuffers();
    }

    SkString getName() const override {
        switch (fType) {
            case Type::kMeshWithImage:      return SkString("mesh_with_image");
            case Type::kMeshWithEffects:    return SkString("mesh_with_effects");
            case Type::kMeshWithPaintColor: return SkString("mesh_with_paint_color");
            case Type::kMeshWithPaintImage: return SkString("mesh_with_paint_image");
            default: SkUNREACHABLE;
        }
    }

    bool onAnimate(double nanos) override {
        // `periodic` goes from zero to 2π every four seconds, then wraps around.
        double periodic = nanos / 4'000'000'000.;
        periodic -= std::floor(periodic);
        periodic *= 2 * SK_DoublePI;

        double xOff[kMeshSize], yOff[kMeshSize];
        for (int index = 0; index < kMeshSize; ++index) {
            xOff[index] = std::sin(periodic) * kRippleSize;
            yOff[index] = std::sin(periodic + 10.0) * kRippleSize;
            periodic += 0.8;
        }

        fVerts.clear();
        for (int y = 0; y < kMeshSize; ++y) {
            float yf = (float)y / (kMeshSize - 1);  // yf = 0 .. 1
            for (int x = 0; x < kMeshSize; ++x) {
                float xf = (float)x / (kMeshSize - 1);  // xf = 0 .. 1

                Vertex* vert = &fVerts.push_back();
                vert->pos[0] = kRect.left() + xf * kRect.width()  + xOff[y];
                vert->pos[1] = kRect.top()  + yf * kRect.height() + yOff[x];
                vert->uv[0]  = kUV.left()   + xf * kUV.width();
                vert->uv[1]  = kUV.top()    + yf * kUV.height();
            }
        }

        return true;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        SkRuntimeEffect::ChildPtr child[4] = {fShader1, fShader2, fColorFilter, fBlender};

        GrRecordingContext* rc = canvas->recordingContext();
        GrDirectContext* dc = GrAsDirectContext(rc);
        fVB->update(dc, fVerts.data(), /*offset=*/0, fVerts.size_bytes());

        SkMesh::Result result = SkMesh::MakeIndexed(fSpec,
                                                    SkMesh::Mode::kTriangles,
                                                    fVB,
                                                    fVerts.size(),
                                                    /*vertexOffset=*/0,
                                                    fIB,
                                                    fIndices.size(),
                                                    /*indexOffset=*/0,
                                                    /*uniforms=*/nullptr,
                                                    /*children=*/child,
                                                    kRect.makeOutset(kRippleSize, kRippleSize));
        if (!result.mesh.isValid()) {
            SkDebugf("Mesh creation failed: %s\n", result.error.c_str());
            return DrawResult::kFail;
        }

        SkPaint paint;
        paint.setShader(fPaintShader);
        canvas->drawMesh(result.mesh, SkBlender::Mode(SkBlendMode::kDstOver), paint);

        return DrawResult::kOk;
    }

private:
    void ensureBuffers() {
        if (!fVB) {
            fVB = SkMeshes::MakeVertexBuffer(fVerts.data(), fVerts.size_bytes());
        }
        if (!fIB) {
            fIB = SkMeshes::MakeIndexBuffer(fIndices.data(), fIndices.size_bytes());
        }
    }

    struct Vertex {
        float pos[2];
        float uv[2];
    };

    static constexpr auto kRect = SkRect::MakeLTRB(20, 20, 300, 300);
    static constexpr auto kUV   = SkRect::MakeLTRB( 0,  0, 128, 128);
    static constexpr int kMeshSize = 16;
    static constexpr float kRippleSize = 6.0f;

    Type fType;

    TArray<Vertex>   fVerts;
    TArray<uint16_t> fIndices;

    sk_sp<SkShader> fShader1, fShader2, fPaintShader;
    sk_sp<SkColorFilter> fColorFilter;
    sk_sp<SkBlender> fBlender;

    sk_sp<SkMeshSpecification> fSpec;

    sk_sp<SkMesh::VertexBuffer> fVB;
    sk_sp<SkMesh::IndexBuffer> fIB;
};

DEF_GM(return new MeshWithShadersGM(MeshWithShadersGM::Type::kMeshWithImage))
DEF_GM(return new MeshWithShadersGM(MeshWithShadersGM::Type::kMeshWithPaintColor))
DEF_GM(return new MeshWithShadersGM(MeshWithShadersGM::Type::kMeshWithPaintImage))
DEF_GM(return new MeshWithShadersGM(MeshWithShadersGM::Type::kMeshWithEffects))

DEF_SIMPLE_GM_CAN_FAIL(custommesh_cs_uniforms, canvas, errorMsg, 200, 900) {
    if (!canvas->recordingContext() && !canvas->recorder()) {
        *errorMsg = GM::kErrorMsg_DrawSkippedGpuOnly;
        return DrawResult::kSkip;
    }

    // Shared data
    static constexpr SkRect kRect = SkRect::MakeLTRB(20, 20, 80, 80);
    static constexpr SkPoint kQuad[]{
            {kRect.left(), kRect.top()},
            {kRect.right(), kRect.top()},
            {kRect.left(), kRect.bottom()},
            {kRect.right(), kRect.bottom()},
    };
    sk_sp<SkMesh::VertexBuffer> vb = SkMeshes::MakeVertexBuffer(kQuad, sizeof(kQuad));
    sk_sp<SkData> unis = SkData::MakeWithCopy(&SkColors::kRed, sizeof(SkColor4f));

    // Surface helper
    auto makeSurface = [=](sk_sp<SkColorSpace> cs) {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(200, 100, cs);
        sk_sp<SkSurface> surface = canvas->makeSurface(ii);
        return surface ? surface : SkSurfaces::Raster(ii);
    };

    // Mesh helper
    enum class Managed : bool { kNo, kYes };
    auto makeMesh = [&](Managed managed, sk_sp<SkColorSpace> workingCS) {
        static const SkMeshSpecification::Attribute kAttributes[]{
                {SkMeshSpecification::Attribute::Type::kFloat2, 0, SkString{"pos"}},
        };

        static constexpr char kVS[] = R"(
            Varyings main(in const Attributes attributes) {
                Varyings varyings;
                varyings.position = attributes.pos;
                return varyings;
            }
        )";
        static constexpr char kManagedFS[] = R"(
            layout(color) uniform half4 color;
            float2 main(const Varyings varyings, out half4 c) {
                c = color;
                return varyings.position;
            }
        )";
        static constexpr char kRawFS[] = R"(
            uniform half4 color;
            float2 main(const Varyings varyings, out half4 c) {
                c = color;
                return varyings.position;
            }
        )";

        auto [spec, error] = SkMeshSpecification::Make(
                kAttributes,
                sizeof(SkPoint),
                /*varyings=*/{},
                SkString(kVS),
                SkString(managed == Managed::kYes ? kManagedFS : kRawFS),
                std::move(workingCS),
                kPremul_SkAlphaType);
        SkASSERT(spec);

        SkMesh::Result result = SkMesh::Make(std::move(spec),
                                             SkMesh::Mode::kTriangleStrip,
                                             vb,
                                             /*vertexCount=*/4,
                                             /*vertexOffset=*/0,
                                             /*uniforms=*/unis,
                                             /*children=*/{},
                                             kRect);
        SkASSERT(result.mesh.isValid());
        return result.mesh;
    };

    sk_sp<SkColorSpace> null = nullptr,
                        srgb = SkColorSpace::MakeSRGB(),
                        spin = SkColorSpace::MakeSRGB()->makeColorSpin(),
                        wide = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2,
                                                     SkNamedGamut::kRec2020);

    struct Config {
        sk_sp<SkColorSpace> fMeshCS;
        sk_sp<SkColorSpace> fSurfaceCS;
        Managed fManaged;
        SkColor fExpectedColor = SK_ColorRED;
    };
    static const Config kConfigs[] = {
            // Uniforms should remain in sRGB mode, then get converted to destination after mesh FS
            // Before b/316594914 was fixed, these would get double-converted:
            {srgb, null, Managed::kYes},
            {srgb, srgb, Managed::kYes},
            {srgb, spin, Managed::kYes},
            {srgb, wide, Managed::kYes},

            // Uniforms should be converted to working space (spun), then converted to destination
            {spin, srgb, Managed::kYes},
            {spin, spin, Managed::kYes},
            {spin, wide, Managed::kYes},

            // Non-managed uniforms serve as a control group. The red uniforms are not converted to
            // the working space. The mesh FS returns "red" {1, 0, 0, 1}, but that's actually green,
            // because the working space of the mesh is `spin`. That output is converted to dest,
            // rendering as green. Therefore, we manually change the control color's box to green.
            {spin, srgb, Managed::kNo, SK_ColorGREEN},
            {spin, wide, Managed::kNo, SK_ColorGREEN},
    };

    for (const Config& config : kConfigs) {
        SkMesh mesh = makeMesh(config.fManaged, config.fMeshCS);

        sk_sp<SkSurface> offscreen = makeSurface(config.fSurfaceCS);
        SkCanvas* offscreenCanvas = offscreen->getCanvas();

        SkPaint paint;
        offscreenCanvas->drawMesh(mesh, SkBlender::Mode(SkBlendMode::kDst), paint);
        offscreenCanvas->translate(100, 0);
        paint.setColor(config.fExpectedColor);
        offscreenCanvas->drawRect(kRect, paint);

        offscreen->draw(canvas, 0, 0);
        canvas->translate(0, 100);
    }

    return DrawResult::kOk;
}

}  // namespace skiagm
