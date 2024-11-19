/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkFont.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkRandom.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "delaunator.hpp"
#include "imgui.h"

namespace {

sk_sp<SkVertices> triangulate_pts(const std::vector<SkPoint>& pts, const std::vector<SkColor>& colors) {
    // put points in the format delaunator wants
    std::vector<double> coords;
    for (size_t i = 0; i < pts.size(); ++i) {
        coords.push_back(pts[i].x());
        coords.push_back(pts[i].y());
    }

    // triangulation happens here
    delaunator::Delaunator d(coords);

    // SkVertices parameters
    std::vector<SkPoint> vertices;
    std::vector<uint16_t> indices;

    // populate vertices & colors
    for(std::size_t i = 0; i < d.coords.size(); i+=2) {
        vertices.push_back(SkPoint::Make(d.coords[i], d.coords[i+1]));
    }

    // populate triangle indices
    for(std::size_t i = 0; i < d.triangles.size(); i+=3) {
        indices.push_back(d.triangles[i]);
        indices.push_back(d.triangles[i+1]);
        indices.push_back(d.triangles[i+2]);
    }

    return SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, vertices.size(), vertices.data(), nullptr, colors.data(), indices.size(), indices.data());
}

sk_sp<SkShader> makeGradientShader(int w, int h, std::vector<SkPoint>& vertices, std::vector<SkColor>& colors) {
    vertices.push_back(SkPoint::Make(0.f, 0.f));
    vertices.push_back(SkPoint::Make(w, 0.f));
    vertices.push_back(SkPoint::Make(0.f, h));
    vertices.push_back(SkPoint::Make(w, h));

    colors.push_back(SK_ColorTRANSPARENT);
    colors.push_back(SK_ColorTRANSPARENT);
    colors.push_back(SK_ColorTRANSPARENT);
    colors.push_back(SK_ColorTRANSPARENT);

    sk_sp<SkVertices> sk_vertices = triangulate_pts(vertices, colors);

    // record with a picture
    SkRect tile = SkRect::MakeWH(w, h);
    SkPictureRecorder recorder;
    SkCanvas* c = recorder.beginRecording(tile);

    SkPaint p;
    p.setColor(SK_ColorWHITE);
    c->drawVertices(sk_vertices, SkBlendMode::kModulate, p);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    return picture->makeShader(SkTileMode::kDecal, SkTileMode::kDecal, SkFilterMode::kNearest);
}

class GradientRenderer : public SkRefCnt {
public:
    virtual void draw(SkCanvas*) const = 0;

    virtual sk_sp<SkShader> asShader() const = 0;

    virtual void updateVertices(SkSpan<const SkPoint> vert_pos,
                                SkSpan<const SkColor4f> vert_colors) = 0;
};

class SkSlRenderer : public GradientRenderer {
public:
    void draw(SkCanvas* canvas) const override {
        SkPaint paint;
        paint.setShader(fShader);
        canvas->drawRect(SkRect::MakeWH(1, 1), paint);
    }

    sk_sp<SkShader> asShader() const override { return fShader; }

    void updateVertices(SkSpan<const SkPoint> vert_pos,
                        SkSpan<const SkColor4f> vert_colors) override {
        SkASSERT(vert_pos.size() == vert_colors.size());
        const auto vert_count = vert_pos.size();

        if (!vert_count) {
            return;
        }

        // Effect compilation is expensive, so we cache and only recompile when the count changes.
        if (vert_count != fCachedCount) {
            this->buildEffect(vert_count);
            fCachedCount = vert_count;
        }

        SkRuntimeEffectBuilder builder(fEffect);
        builder.uniform("u_vertcolors").set(vert_colors.data(), vert_colors.size());
        builder.uniform("u_vertpos")   .set(vert_pos.data()   , vert_pos.size());

        fShader = builder.makeShader();
    }
    virtual void buildEffect(size_t vert_count) = 0;

protected:
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkShader>        fShader;

    size_t                 fCachedCount = 0;
};

class AEGradientRenderer final : public SkSlRenderer {
public:
    void buildEffect(size_t vert_count) override {
        static constexpr char gAEGradientSkSL[] =
            "uniform half4  u_vertcolors[%zu];"
            "uniform float2 u_vertpos[%zu];"

            "half4 main(float2 xy) {"
                "half4 c = half4(0);"
                "float w_acc = 0;"

                "for (int i = 0; i < %zu; ++i) {"
                    "float d = distance(xy, u_vertpos[i]);"
                    "float w = 1 / (d * d);"

                    "c += u_vertcolors[i] * w;"
                    "w_acc += w;"
                "}"

                "return c / w_acc;"
            "}";

        const auto res = SkRuntimeEffect::MakeForShader(
                            SkStringPrintf(gAEGradientSkSL, vert_count, vert_count, vert_count));
        if (!res.effect) {
            SkDEBUGF("%s\n", res.errorText.c_str());
        }

        fEffect = res.effect;

        SkASSERT(fEffect);
    }
};

class LinearGradientRenderer final : public SkSlRenderer {
public:
    void buildEffect(size_t vert_count) override {
        static constexpr char sksl[] =
            "uniform half4  u_vertcolors[%zu];"
            "uniform float2 u_vertpos[%zu];"

            "half4 main(float2 xy) {"
                "float v[%zu];"
                "for (int i = 0; i < %zu; i++) {"
                    "v[i] = 1.;"
                "}"

                "for (int i = 0; i < %zu; ++i) {"
                    "for (int j = 0; j < %zu; ++j) {"
                        "vec2 delta;"
                        "delta.x = u_vertpos[j].x - u_vertpos[i].x;"
                        "delta.y = u_vertpos[j].y - u_vertpos[i].y;"

                        "mat3 m = mat3 ("
                            "delta.x, delta.y, 0.,"                 // 1st column
                            "-delta.y, delta.x, 0.,"                // 2nd column
                            "u_vertpos[i].x, u_vertpos[i].y, 1."    // 3rd column
                        ");"
                        "mat3 m_inv = inverse(m);"

                        "vec3 p_h = vec3(xy.x, xy.y, 1.);"
                        "vec3 u = m_inv*p_h;"
                        "float t = u.x;"

                        "if (t < 0) {"
                            "v[j] = 0;"
                        "} else if (t > 1) {"
                            "v[i] = 0;"
                        "} else {"
                            "v[i] *= 1-t;"
                            "v[j] *= t;"
                        "}"
                    "}"
                "}"

                "half4 c = half4(0);"
                "float w_acc = 0;"
                "for (int i = 0; i < %zu; i++) {"
                    "c += u_vertcolors[i] * v[i];"
                    "w_acc += v[i];"
                "}"

                "return c / w_acc;"
            "}";

        const auto res = SkRuntimeEffect::MakeForShader(
                            SkStringPrintf(sksl, vert_count, vert_count, vert_count, vert_count, vert_count, vert_count, vert_count));
        if (!res.effect) {
            SkDEBUGF("%s\n", res.errorText.c_str());
        }

        fEffect = res.effect;

        SkASSERT(fEffect);
    }
};

class IllGradientRenderer final : public SkSlRenderer {
public:
    void buildEffect(size_t vert_count) override {
        static constexpr char sksl[] =
            "uniform half4  u_vertcolors[%zu];"
            "uniform float2 u_vertpos[%zu];"

            "half4 main(float2 xy) {"
                "float d[%zu];"
                "for (int i = 0; i < %zu; i++) {"
                    "d[i] = 0.;"
                "}"

                "for (int i = 0; i < %zu; ++i) {"
                    "for (int j = 0; j < %zu; ++j) {"
                        "vec2 delta;"
                        "delta.x = u_vertpos[j].x - u_vertpos[i].x;"
                        "delta.y = u_vertpos[j].y - u_vertpos[i].y;"

                        "mat3 m = mat3 ("
                            "delta.x, delta.y, 0.,"                 // 1st column
                            "-delta.y, delta.x, 0.,"                // 2nd column
                            "u_vertpos[i].x, u_vertpos[i].y, 1."    // 3rd column
                        ");"
                        "mat3 m_inv = inverse(m);"

                        "vec3 p_h = vec3(xy.x, xy.y, 1.);"
                        "vec3 u = m_inv*p_h;"
                        "float t = u.x;"

                        "float s = length(delta);"
                        "if (t < 0) {"
                            "d[i] += s*abs(u.y);"
                            "d[j] += s*distance(vec2(u.x, u.y), vec2(1., 0.));"
                        "} else if (t > 1) {"
                            "d[j] += s*abs(u.y);"
                            "d[i] += s*distance(vec2(u.x, u.y), vec2(0., 0.));"
                        "} else {"
                            "d[i] += s*distance(vec2(u.x, u.y), vec2(0., 0.));"
                            "d[j] += s*distance(vec2(u.x, u.y), vec2(1., 0.));"
                        "}"
                    "}"
                "}"

                "half4 c = half4(0);"
                "float w_acc = 0;"
                "for (int i = 0; i < %zu; i++) {"
                    "float w = 1 / (d[i] * d[i]);"
                    "c += u_vertcolors[i] * w;"
                    "w_acc += w;"
                "}"

                "return c / w_acc;"
            "}";

        const auto res = SkRuntimeEffect::MakeForShader(
                            SkStringPrintf(sksl, vert_count, vert_count, vert_count, vert_count, vert_count, vert_count, vert_count));
        if (!res.effect) {
            SkDEBUGF("%s\n", res.errorText.c_str());
        }

        fEffect = res.effect;

        SkASSERT(fEffect);
    }
};

class TriangulatedGradientRenderer final : public GradientRenderer {
public:
    void draw(SkCanvas* canvas) const override {
        SkPaint paint;
        paint.setShader(fShader);
        canvas->drawRect(SkRect::MakeWH(1, 1), paint);
    }

    sk_sp<SkShader> asShader() const override { return fShader; }

    void updateVertices(SkSpan<const SkPoint> vert_pos,
                        SkSpan<const SkColor4f> vert_colors) override {
        SkASSERT(vert_pos.size() == vert_colors.size());
        const auto vert_count = vert_pos.size();

        if (!vert_count) {
            return;
        }

        std::vector<SkPoint> pos;
        for (auto& p : vert_pos) {
            pos.push_back(p);
        }
        std::vector<SkColor> colors;
        for (auto& c : vert_colors) {
            colors.push_back(c.toSkColor());
        }

        fShader = makeGradientShader(1, 1, pos, colors);
    }

private:
    sk_sp<SkShader> fShader;
};

static constexpr struct RendererChoice {
    const char* fName;
    sk_sp<GradientRenderer>(*fFactory)();
} gGradientRenderers[] = {
    {
        "AfterEffects Gradient",
        []() -> sk_sp<GradientRenderer> { return sk_make_sp<AEGradientRenderer>(); }
    },
    {
        "n-Linear gradient",
        []() -> sk_sp<GradientRenderer> { return sk_make_sp<LinearGradientRenderer>(); }
    },
    {
        "Illustrator (attempt) gradient",
        []() -> sk_sp<GradientRenderer> { return sk_make_sp<IllGradientRenderer>(); }
    },
    {
        "Triangulated gradient",
        []() -> sk_sp<GradientRenderer> { return sk_make_sp<TriangulatedGradientRenderer>(); }
    }
};

float lerp(float a, float b, float t) {
    return a + (b - a)*t;
}

static constexpr struct VertexAnimator {
    const char* fName;
    void (*fAanimate)(SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t);
} gVertexAnimators[] = {
    {
        "Wigglynator",
        [](SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t) {
            const float radius = t*0.2f/(std::sqrt(uvs.size()) - 1);
            for (size_t i = 0; i < uvs.size(); ++i) {
                const float phase = i*SK_FloatPI*0.31f,
                            angle = phase + t*SK_FloatPI*2;
                pos[i] = uvs[i] + SkVector{
                    radius*std::cos(angle),
                    radius*std::sin(angle),
                };
            }
        },
    },
    {
        "Squircillator",
        // Pull all vertices towards center, proportionally, such that the outer square edge
        // is mapped to a circle for t == 1.
        [](SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t) {
            for (size_t i = 0; i < uvs.size(); ++i) {
                // remap to [-.5,.5]
                const auto uv = (uvs[i] - SkPoint{0.5,0.5});
                // can't allow len to collapse to zero, lest bad things happen at {0.5, 0.5}
                const auto len = std::max(uv.length(), std::numeric_limits<float>::min());

                // Distance from center to outer edge for the line pasing through uv.
                const auto d = len*0.5f/std::max(std::abs(uv.fX), std::abs(uv.fY));
                // Scale needed to pull the outer edge to the r=0.5 circle at t == 1.
                const auto s = lerp(1, (0.5f / d), t);

                pos[i] = uv*s + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Twirlinator",
        // Rotate vertices proportional to their distance to center.
        [](SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t) {
            static constexpr float kMaxRotate = SK_FloatPI*4;

            for (size_t i = 0; i < uvs.size(); ++i) {
                // remap to [-.5,.5]
                const auto uv = (uvs[i] - SkPoint{0.5,0.5});
                const auto angle = kMaxRotate * t * uv.length();

                pos[i] = SkMatrix::RotateRad(angle).mapPoint(uv) + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Cylinderator",
        // Simulate a cylinder rolling sideways across the 1x1 uv space.
        [](SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t) {
            static constexpr float kCylRadius = .2f;

            const auto cyl_pos = t;

            for (size_t i = 0; i < uvs.size(); ++i) {
                if (uvs[i].fX <= cyl_pos) {
                    pos[i] = uvs[i];
                    continue;
                }

                const auto arc_len = uvs[i].fX - cyl_pos,
                           arc_ang = arc_len/kCylRadius;

                pos[i] = SkPoint{
                    cyl_pos + std::sin(arc_ang)*kCylRadius,
                    uvs[i].fY,
                };
            }
        },
    },
    {
        "None",
        [](SkSpan<const SkPoint> uvs, SkSpan<SkPoint> pos, float t) {
            memcpy(pos.data(), uvs.data(), uvs.size() * sizeof(SkPoint));
        },
    },
};

class MeshGradientSlide final : public Slide {
public:
    MeshGradientSlide()
        : fCurrentRenderer(gGradientRenderers[0].fFactory())
        , fFont(ToolUtils::DefaultPortableTypeface(), .75f)
        , fTimeMapper({0.5f, 0}, {0.5f, 1})
        , fVertedCountTimeMapper({0, 0.5f}, {0.5f, 1})
    {
            fName = "MeshGradient";
    }

    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);

        static constexpr float kMeshViewFract = 0.85f;
        const float mesh_size = std::min(fSize.fWidth, fSize.fHeight) * kMeshViewFract;

        canvas->translate((fSize.fWidth  - mesh_size) * 0.5f,
                          (fSize.fHeight - mesh_size) * 0.5f);
        canvas->scale(mesh_size, mesh_size);

        if (fShaderFill) {
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setShader(fCurrentRenderer->asShader());
            canvas->drawString("SK", 0, 0.75f, fFont, paint);
        } else {
            fCurrentRenderer->draw(canvas);
        }

        this->drawControls();
    }

    bool animate(double nanos) override {
        // Mesh count animation
        {
            if (!fVertexCountTimeBase) {
                fVertexCountTimeBase = nanos;
            }

            static constexpr float kSizeAdjustSeconds = 2;
            const float t = (nanos - fVertexCountTimeBase) * 0.000000001 / kSizeAdjustSeconds;

            this->updateMesh(lerp(fVertexCountFrom,
                                  fVertexCountTo,
                                  fVertedCountTimeMapper.computeYFromX(t)));
        }

        // Vertex animation
        {
            if (!fTimeBase) {
                fTimeBase = nanos;
            }

            const float t =
                std::abs((std::fmod((nanos - fTimeBase) * 0.000000001 * fAnimationSpeed, 2) - 1));

            fCurrentAnimator->fAanimate(fVertUVs, fVertPos, fTimeMapper.computeYFromX(t));

            fCurrentRenderer->updateVertices(fVertPos, fVertColors);
        }

        return true;
    }

private:
    void updateMesh(size_t new_count) {
        // These look better than rng when the count is low.
        static constexpr struct {
            SkPoint   fUv;
            SkColor4f fColor;
        } gFixedVertices[] = {
            {{ .25f, .25f}, {1, 0, 0, 1}},
            {{ .75f, .75f}, {0, 1, 0, 1}},
            {{ .75f, .25f}, {0, 0, 1, 1}},
            {{ .25f, .75f}, {1, 1, 0, 1}},
            {{ .50f, .50f}, {0, 1, 1, 1}},
        };

        SkASSERT(fVertUVs.size() == fVertPos.size());
        SkASSERT(fVertUVs.size() == fVertColors.size());
        const size_t current_count = fVertUVs.size();

        fVertUVs.resize(new_count);
        fVertPos.resize(new_count);
        fVertColors.resize(new_count);

        for (size_t i = current_count; i < new_count; ++i) {
            const SkPoint uv = i < std::size(gFixedVertices)
                ? gFixedVertices[i].fUv
                : SkPoint{ fRNG.nextF(), fRNG.nextF() };
            const SkColor4f color = i < std::size(gFixedVertices)
                ? gFixedVertices[i].fColor
                : SkColor4f{ fRNG.nextF(), fRNG.nextF(), fRNG.nextF(), 1.f };

            fVertUVs[i] = fVertPos[i] = uv;
            fVertColors[i] = color;
        }

        if (new_count < current_count) {
            // Reset the RNG state
            fRNG.setSeed(0);
            static constexpr size_t kRandsPerVertex = 5;
            const size_t rand_vertex_count =
                std::max(new_count, std::size(gFixedVertices)) - std::size(gFixedVertices);
            for (size_t i = 0; i < rand_vertex_count * kRandsPerVertex; ++i) { fRNG.nextF(); }
        }
    }

    void drawControls() {
        ImGui::Begin("Mesh Options");

        if (ImGui::BeginCombo("Gradient Type", fCurrentRendererChoice->fName)) {
            for (const auto& renderer : gGradientRenderers) {
                const auto is_selected = (fCurrentRendererChoice->fName == renderer.fName);
                if (ImGui::Selectable(renderer.fName) && !is_selected) {
                    fCurrentRendererChoice = &renderer;
                    fCurrentRenderer = renderer.fFactory();
                    fCurrentRenderer->updateVertices(fVertPos, fVertColors);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Animator", fCurrentAnimator->fName)) {
            for (const auto& anim : gVertexAnimators) {
                const auto is_selected = (fCurrentAnimator->fName == anim.fName);
                if (ImGui::Selectable(anim.fName) && !is_selected) {
                    fCurrentAnimator = &anim;
                    fTimeBase = 0;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::SliderInt("Vertex Count", &fVertexCountTo, 3, 64)) {
            fVertexCountTimeBase = 0;
            fVertexCountFrom = fVertUVs.size();
        }

        ImGui::SliderFloat("Speed", &fAnimationSpeed, 0.25, 4, "%.2f");

        ImGui::Checkbox("Shader Fill", &fShaderFill);

        ImGui::End();
    }

    SkSize                  fSize;
    std::vector<SkPoint>    fVertUVs;
    std::vector<SkPoint>    fVertPos;    // fVertUVs after animation
    std::vector<SkColor4f>  fVertColors;

    SkRandom                fRNG;

    sk_sp<GradientRenderer> fCurrentRenderer;

    const SkFont            fFont;

    // Animation state
    const SkCubicMap        fTimeMapper,
                            fVertedCountTimeMapper;
    double                  fTimeBase               = 0,
                            fVertexCountTimeBase    = 0;
    int                     fVertexCountFrom        = 0,
                            fVertexCountTo          = 5;

    // UI stuff
    const RendererChoice*   fCurrentRendererChoice  = &gGradientRenderers[0];
    const VertexAnimator*   fCurrentAnimator        = &gVertexAnimators[0];
    float                   fAnimationSpeed         = 1.f;
    bool                    fShaderFill             = false;
};

}  // anonymous namespace

DEF_SLIDE(return new MeshGradientSlide{};)
