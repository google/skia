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
#include "include/core/SkData.h"
#include "include/core/SkMesh.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkRandom.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkMeshPriv.h"
#include "tools/timer/TimeUtils.h"

#include <memory>

namespace skiagm {
class MeshGM : public skiagm::GM {
public:
    MeshGM() {}

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

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

                    Varyings main(const in Attributes attributes) {
                        Varyings varyings;
                        varyings.color    = unswizzle_color(attributes.brag);
                        varyings.uv       = attributes.xuyv.yw;
                        varyings.position = attributes.xuyv.xz;
                        return varyings;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 main(const in Varyings varyings, out float4 color) {
                        color = varyings.color;
                        return varyings.uv;
                    }
            )";
            auto[spec, error] =
                    SkMeshSpecification::Make(kAttributes,
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
            auto[spec, error] =
                    SkMeshSpecification::Make(kAttributes,
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

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* string) override {
        auto dc = GrAsDirectContext(canvas->recordingContext());
        this->ensureBuffers();
        if (!dc || dc->abandoned()) {
            return DrawResult::kOk;
        }

        fColorVB        = SkMesh::CopyVertexBuffer(dc, fColorVB);
        fColorIndexedVB = SkMesh::CopyVertexBuffer(dc, fColorIndexedVB);
        fIB[1]          = SkMesh::CopyIndexBuffer (dc, fIB[0]);
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
                SkMesh::Result result;
                // Rather than pile onto the combinatorics we draw every other test case indexed.
                if ((i & 1) == 0) {
                    if (colors) {
                        result = SkMesh::Make(fSpecWithColor,
                                            SkMesh::Mode::kTriangleStrip,
                                            fColorVB,
                                            /*vertexCount= */4,
                                            /*vertexOffset=*/0,
                                            /*uniforms=    */nullptr,
                                            kRect);
                    } else {
                        result = SkMesh::Make(fSpecWithNoColor,
                                            SkMesh::Mode::kTriangleStrip,
                                            fNoColorVB,
                                            /*vertexCount=*/4,
                                            kNoColorOffset,
                                            /*uniforms=*/nullptr,
                                            kRect);
                    }
                } else {
                    // Alternate between CPU and GPU-backend index buffers.
                    auto ib = (i%4 == 0) ? fIB[0] : fIB[1];
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
            fColorVB = SkMesh::MakeVertexBuffer(/*GrDirectContext*=*/nullptr,
                                                kColorQuad,
                                                sizeof(kColorQuad));
        }

        if (!fNoColorVB) {
            // Make this one such that the data is offset into the buffer.
            auto data = SkData::MakeUninitialized(sizeof(kNoColorQuad) + kNoColorOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kNoColorOffset),
                        kNoColorQuad,
                        sizeof(kNoColorQuad));
            fNoColorVB = SkMesh::MakeVertexBuffer(/*GrDirectContext*=*/nullptr,
                                                  data->data(),
                                                  data->size());
        }

        if (!fColorIndexedVB) {
            // This buffer also has an offset.
            auto data = SkData::MakeUninitialized(sizeof(kColorIndexedQuad) + kColorIndexedOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kColorIndexedOffset),
                        kColorIndexedQuad,
                        sizeof(kColorIndexedQuad));
            fColorIndexedVB = SkMesh::MakeVertexBuffer(/*GrDirectContext*=*/nullptr,
                                                       data->data(),
                                                       data->size());
        }

        if (!fNoColorIndexedVB) {
            fNoColorIndexedVB = SkMesh::MakeVertexBuffer(/*GrDirectContext*=*/nullptr,
                                                         kNoColorIndexedQuad,
                                                         sizeof(kNoColorIndexedQuad));
        }

        if (!fIB[0]) {
            // The index buffer has an offset.
            auto data = SkData::MakeUninitialized(sizeof(kIndices) + kIndexOffset);
            std::memcpy(SkTAddOffset<void>(data->writable_data(), kIndexOffset),
                        kIndices,
                        sizeof(kIndices));
            fIB[0] = SkMesh::MakeIndexBuffer(/*GrDirectContext*=*/nullptr,
                                             data->data(),
                                             data->size());
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

        fVB = SkMesh::MakeVertexBuffer(nullptr, kQuad, sizeof(kQuad));
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
                auto result = SkMesh::Make(fSpecs[SpecIndex(unpremul, spin)],
                                           SkMesh::Mode::kTriangleStrip,
                                           fVB,
                                           /*vertexCount= */4,
                                           /*vertexOffset=*/0,
                                           /*uniforms=    */nullptr,
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

DEF_GM(return new MeshColorSpaceGM;)

class MeshUniformsGM : public skiagm::GM {
public:
    MeshUniformsGM() { this->onAnimate(0); }

protected:
    using Attribute = SkMeshSpecification::Attribute;
    using Varying   = SkMeshSpecification::Varying;

    SkISize onISize() override { return {140, 250}; }

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

        fVB = SkMesh::MakeVertexBuffer(nullptr, kQuad, sizeof(kQuad));
    }

    SkString onShortName() override { return SkString("custommesh_uniforms"); }

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
                                       /*vertexCount= */4,
                                       /*vertexOffset=*/0,
                                       /*uniforms=    */std::move(unis),
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

    SkISize onISize() override { return {270, 490}; }

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
        fShader = SkImage::MakeRasterCopy(pixmap)->makeShader(
                SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions{SkFilterMode::kLinear});
    }

    SkString onShortName() override { return SkString("mesh_updates"); }

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

            auto vb =
                    SkMesh::MakeVertexBuffer(ctx, /*data=*/nullptr, kVBRects*6*sizeof(Vertex));

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
                                           /*vertexCount=*/6*rectCount,
                                           /*vertexOffset=*/0,
                                           nullptr,
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
            vb = SkMesh::MakeVertexBuffer(
                    ctx, /*data=*/nullptr, kNumIBUpdates*4*sizeof(Vertex));
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

            auto ib =
                    SkMesh::MakeIndexBuffer(ctx, /*data=*/nullptr, kIBRects*6*sizeof(uint16_t));

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
                                                  /*vertexCount= */ 4*kNumIBUpdates,
                                                  /*vertexOffset=*/0,
                                                  ib,
                                                  /*indexCount= */ 6,
                                                  /*indexOffset=*/offset,
                                                  /*uniforms=   */ nullptr,
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

    SkISize onISize() override { return {90, 30}; }

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

    SkString onShortName() override { return SkString("mesh_zero_init"); }

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
                auto vb = SkMesh::MakeVertexBuffer(ctx, nullptr, spec->stride()*std::size(kTri));
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
                auto ib = SkMesh::MakeIndexBuffer(ctx, nullptr, sizeof(uint16_t)*4);
                SkAssertResult(ib->update(ctx, kTiIndices, indexUploadOffset, sizeof(kTiIndices)));

                SkRect bounds;
                bounds.setBounds(kTri, std::size(kTri));
                auto result = SkMesh::MakeIndexed(spec,
                                                  SkMesh::Mode::kTriangles,
                                                  std::move(vb),
                                                  /*vertexCount=*/ std::size(kTri),
                                                  /*vertexOffset=*/0,
                                                  std::move(ib),
                                                  /*indexCount=*/std::size(kTiIndices) + 1,
                                                  indexMeshOffset,
                                                  /*uniforms=*/nullptr,
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
                    ctx->flushAndSubmit(true);
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

    SkISize onISize() override { return {390, 90}; }

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

        fVB = SkMesh::MakeVertexBuffer(nullptr, kQuad, sizeof(kQuad));
        fIB = SkMesh::MakeIndexBuffer(nullptr, kIndices, sizeof(kIndices));

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

    SkString onShortName() override { return SkString("picture_mesh"); }

    DrawResult onDraw(SkCanvas* canvas, SkString* error) override {
        SkPaint paint;
        paint.setShader(fShader);

        auto dc = GrAsDirectContext(canvas->recordingContext());
        for (bool picture : {false, true}) {
            canvas->save();
            for (bool gpu : {false, true}) {
                auto vb = gpu ? SkMesh::CopyVertexBuffer(dc, fVB) : fVB;
                auto ib = gpu ? SkMesh::CopyIndexBuffer (dc, fIB) : fIB;

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
                                             1*sizeof(Vertex),
                                             std::move(uniforms),
                                             kRect);
                            break;
                        case 1:
                            r = SkMesh::Make(fSpec,
                                             SkMesh::Mode::kTriangleStrip,
                                             fVB,
                                             4,
                                             1*sizeof(Vertex),
                                             std::move(uniforms),
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
                                                    2*(sizeof(uint16_t)),
                                                    std::move(uniforms),
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
                                                    2*sizeof(uint16_t),
                                                    std::move(uniforms),
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

}  // namespace skiagm
