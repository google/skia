/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkFloatingPoint.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

#include "imgui.h"

namespace {

float lerp(float a, float b, float t) {
    return a + (b - a)*t;
}

sk_sp<SkShader> make_image_shader(const char* resource) {
    sk_sp<SkImage> img = ToolUtils::GetResourceAsImage(resource);

    // Normalize to 1x1 for UV sampling.
    const auto lm = SkMatrix::Scale(1.0f/img->width(), 1.0f/img->height());

    return img->makeShader(SkTileMode::kDecal,
                           SkTileMode::kDecal,
                           SkSamplingOptions(SkFilterMode::kLinear),
                           &lm);
}

static constexpr struct ShaderFactory {
    const char*     fName;
    sk_sp<SkShader> (*fBuild)();
} gShaderFactories[] = {
    {
        "Img (Mandrill)",
        []() ->sk_sp<SkShader> { return make_image_shader("images/mandrill_512.png"); }
    },
    {
        "Img (Baby Tux)",
        []() ->sk_sp<SkShader> { return make_image_shader("images/baby_tux.png"); }
    },
    {
        "Img (Brickwork)",
        []() ->sk_sp<SkShader> { return make_image_shader("images/brickwork-texture.jpg"); }
    },
    {
        "Radial Gradient",
        []() ->sk_sp<SkShader> {
            static constexpr SkColor gColors[] = {
                SK_ColorGREEN, SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN
            };
            return SkGradientShader::MakeRadial({0.5f, 0.5f}, 0.5f, gColors, nullptr,
                                                std::size(gColors), SkTileMode::kRepeat);
        }
    },
    {
        "Colors",
        []() ->sk_sp<SkShader> { return nullptr; }
    },
};

static constexpr struct VertexAnimator {
    const char* fName;
    void (*fAanimate)(const std::vector<SkPoint>& uv, float t, std::vector<SkPoint>& out);
} gVertexAnimators[] = {
    {
        "Cylinderator",
        // Simulate a cylinder rolling sideways across the 1x1 uv space.
        [](const std::vector<SkPoint>& uvs, float t, std::vector<SkPoint>& out) {
            static constexpr float kCylRadius = .2f;

            const auto cyl_pos = t;

            for (size_t i = 0; i < uvs.size(); ++i) {
                const auto& uv = uvs[i];

                if (uv.fX <= cyl_pos) {
                    out[i] = uv;
                    continue;
                }

                const auto arc_len = uv.fX - cyl_pos,
                           arc_ang = arc_len/kCylRadius;

                out[i] = SkPoint{
                    cyl_pos + std::sin(arc_ang)*kCylRadius,
                    uv.fY,
                };
            }
        },
    },
    {
        "Squircillator",
        // Pull all vertices towards center, proportionally, such that the outer square edge
        // is mapped to a circle for t == 1.
        [](const std::vector<SkPoint>& uvs, float t, std::vector<SkPoint>& out) {
            for (size_t i = 0; i < uvs.size(); ++i) {
                // remap to [-.5,.5]
                const auto uv = (uvs[i] - SkPoint{0.5,0.5});

                // Distance from center to outer edge for the line pasing through uv.
                const auto d = uv.length()*0.5f/std::max(std::abs(uv.fX), std::abs(uv.fY));
                // Scale needed to pull the outer edge to the r=0.5 circle at t == 1.
                const auto s = lerp(1, (0.5f / d), t);

                out[i] = uv*s + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Twirlinator",
        // Rotate vertices proportional to their distance to center.
        [](const std::vector<SkPoint>& uvs, float t, std::vector<SkPoint>& out) {
            static constexpr float kMaxRotate = SK_FloatPI*4;

            for (size_t i = 0; i < uvs.size(); ++i) {
                // remap to [-.5,.5]
                const auto uv = (uvs[i] - SkPoint{0.5,0.5});
                const auto angle = kMaxRotate * t * uv.length();

                out[i] = SkMatrix::RotateRad(angle).mapPoint(uv) + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Wigglynator",
        [](const std::vector<SkPoint>& uvs, float t, std::vector<SkPoint>& out) {
            const float radius = t*0.2f/(std::sqrt(uvs.size()) - 1);
            for (size_t i = 0; i < uvs.size(); ++i) {
                const float phase = i*SK_FloatPI*0.31f,
                            angle = phase + t*SK_FloatPI*2;
                out[i] = uvs[i] + SkVector{
                    radius*std::cos(angle),
                    radius*std::sin(angle),
                };
            }
        },
    },
    {
        "None",
        [](const std::vector<SkPoint>& uvs, float, std::vector<SkPoint>& out) { out = uvs; },
    },
};

class MeshSlide final : public Slide {
public:
    MeshSlide() : fTimeMapper({0.5f, 0}, {0.5f, 1}) { fName = "Mesh"; }

    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};

        this->initMesh(256);
        this->initShader(gShaderFactories[0]);
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SK_ColorWHITE);

        static constexpr float kMeshFraction = 0.85f;
        const float mesh_size = std::min(fSize.fWidth, fSize.fHeight)*kMeshFraction;

        canvas->translate((fSize.fWidth  - mesh_size) * 0.5f,
                          (fSize.fHeight - mesh_size) * 0.5f);
        canvas->scale(mesh_size, mesh_size);

        auto verts = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                          fVertices.size(),
                                          fVertices.data(),
                                          fShader ? fUVs.data() : nullptr,
                                          fShader ? nullptr : fColors.data(),
                                          fIndices.size(),
                                          fIndices.data());
        p.setShader(fShader);
        canvas->drawVertices(verts, SkBlendMode::kModulate, p);

        if (fShowMesh) {
            p.setShader(nullptr);
            p.setColor(SK_ColorBLUE);
            p.setStroke(true);
            p.setStrokeWidth(0.5f/mesh_size);

            SkASSERT(fIndices.size() % 6 == 0);
            for (auto i = fIndices.cbegin(); i < fIndices.cend(); i += 6) {
                canvas->drawLine(fVertices[i[0]], fVertices[i[1]], p);
                canvas->drawLine(fVertices[i[1]], fVertices[i[2]], p);
                canvas->drawLine(fVertices[i[2]], fVertices[i[0]], p);
                canvas->drawLine(fVertices[i[3]], fVertices[i[4]], p);
                canvas->drawLine(fVertices[i[4]], fVertices[i[5]], p);
                canvas->drawLine(fVertices[i[5]], fVertices[i[3]], p);
            }

            p.setStrokeCap(SkPaint::kRound_Cap);
            p.setStrokeWidth(5/mesh_size);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, fVertices.size(), fVertices.data(), p);
        }

        this->drawControls();
    }

    bool animate(double nanos) override {
        if (!fTimeBase) {
            fTimeBase = nanos;
        }

        // Oscillating between 0..1
        const float t =
                std::abs((std::fmod((nanos - fTimeBase)*0.000000001*fAnimationSpeed, 2) - 1));

        // Add some easing
        fCurrentAnimator->fAanimate(fUVs, fTimeMapper.computeYFromX(t), fVertices);

        return true;
    }

private:
    void initMesh(size_t vertex_count) {
        // Generate an NxN mesh.  For simplicity, we keep the vertices in normalized space
        // (1x1 same as UVs), and scale the mesh up when rendering.
        const auto n = static_cast<size_t>(std::sqrt(vertex_count));
        SkASSERT(n > 0);
        SkASSERT(n == std::sqrt(vertex_count));

        fVertices.resize(vertex_count);
        fUVs.resize(vertex_count);
        fColors.resize(vertex_count);
        for (size_t i = 0; i < vertex_count; ++i) {
            fVertices[i] = fUVs[i] = {
                static_cast<float>(i % n) / (n - 1),
                static_cast<float>(i / n) / (n - 1),
            };
            fColors[i] = SkColorSetRGB(!!(i%2)*255,
                                       !!(i%3)*255,
                                       !!((i+1)%3)*255);
        }

        // Trivial triangle tessellation pattern:
        //
        // *---*---*
        // |  /|\  |
        // | / | \ |
        // |/  |  \|
        // *---*---*
        // |\  |  /|
        // | \ | / |
        // |  \|/  |
        // *---*---*
        const size_t triangle_count = 2*(n - 1)*(n - 1),
                        index_count = 3*triangle_count;

        fIndices.clear();
        fIndices.reserve(index_count);
        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = 0; j < n - 1; ++j) {
                const auto row_0_idx = j*n + i,
                           row_1_idx = row_0_idx + n,
                           off_0 = (i + j) % 2,
                           off_1 = 1 - off_0;

                fIndices.push_back(row_0_idx +     0);
                fIndices.push_back(row_0_idx +     1);
                fIndices.push_back(row_1_idx + off_0);

                fIndices.push_back(row_0_idx + off_1);
                fIndices.push_back(row_1_idx +     1);
                fIndices.push_back(row_1_idx +     0);
            }
        }

        SkASSERT(fIndices.size() == index_count);
    }

    void initShader(const ShaderFactory& fact) {
        fShader = fact.fBuild();
        fCurrentShaderFactory = &fact;
    }

    void drawControls() {
        ImGui::Begin("Mesh Options");

        if (ImGui::BeginCombo("Texture", fCurrentShaderFactory->fName)) {
            for (const auto& fact : gShaderFactories) {
                const auto is_selected = (fCurrentShaderFactory->fName == fact.fName);
                if (ImGui::Selectable(fact.fName) && !is_selected) {
                    this->initShader(fact);
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

        static constexpr struct {
            const char* fLabel;
            size_t      fCount;
        } gSizeInfo[] = {
            {     "4x4",    16 },
            {     "8x8",    64 },
            {   "16x16",   256 },
            {   "32x32",  1024 },
            {   "64x64",  4096 },
            { "128x128", 16384 },
        };
        ImGui::SliderInt("Mesh Size",
                         &fMeshSizeSelector,
                         0, std::size(gSizeInfo) - 1,
                         gSizeInfo[fMeshSizeSelector].fLabel);
        if (fVertices.size() != gSizeInfo[fMeshSizeSelector].fCount) {
            this->initMesh(gSizeInfo[fMeshSizeSelector].fCount);
        }

        ImGui::SliderFloat("Speed", &fAnimationSpeed, 0.25, 4, "%.2f");

        ImGui::Checkbox("Show mesh", &fShowMesh);

        ImGui::End();
    }

    SkSize                  fSize;
    sk_sp<SkShader>         fShader;
    std::vector<SkPoint>    fVertices,
                            fUVs;
    std::vector<SkColor>    fColors;
    std::vector<uint16_t>   fIndices;

    double                  fTimeBase = 0;
    const SkCubicMap        fTimeMapper;

    // UI stuff
    const ShaderFactory*    fCurrentShaderFactory = &gShaderFactories[0];
    const VertexAnimator*   fCurrentAnimator      = &gVertexAnimators[0];
    int                     fMeshSizeSelector     = 2;
    float                   fAnimationSpeed       = 1.f;
    bool                    fShowMesh             = false;
};

}  // anonymous namespace

DEF_SLIDE(return new MeshSlide{};)
