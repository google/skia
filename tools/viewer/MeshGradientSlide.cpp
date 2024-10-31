/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkPoint.h"
#include "src/base/SkRandom.h"
#include "tools/viewer/Slide.h"

#include <cmath>
#include <cstddef>
#include <vector>

#include "imgui.h"

namespace {

struct ColorVertex {
    SkPoint   fUV,    // position, in normalized [0..1] space
              fPos;   // draw position (post animation), in normalized space
    SkColor4f fColor;
};

float lerp(float a, float b, float t) {
    return a + (b - a)*t;
}

static constexpr struct VertexAnimator {
    const char* fName;
    void (*fAanimate)(std::vector<ColorVertex>& verts, float t);
} gVertexAnimators[] = {
    {
        "Wigglynator",
        [](std::vector<ColorVertex>& verts, float t) {
            const float radius = t*0.2f/(std::sqrt(verts.size()) - 1);
            for (size_t i = 0; i < verts.size(); ++i) {
                const float phase = i*SK_FloatPI*0.31f,
                            angle = phase + t*SK_FloatPI*2;
                verts[i].fPos = verts[i].fUV + SkVector{
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
        [](std::vector<ColorVertex>& verts, float t) {
            for (auto& v : verts) {
                // remap to [-.5,.5]
                const auto uv = (v.fUV - SkPoint{0.5,0.5});

                // Distance from center to outer edge for the line pasing through uv.
                const auto d = uv.length()*0.5f/std::max(std::abs(uv.fX), std::abs(uv.fY));
                // Scale needed to pull the outer edge to the r=0.5 circle at t == 1.
                const auto s = lerp(1, (0.5f / d), t);

                v.fPos = uv*s + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Twirlinator",
        // Rotate vertices proportional to their distance to center.
        [](std::vector<ColorVertex>& verts, float t) {
            static constexpr float kMaxRotate = SK_FloatPI*4;

            for (auto& v : verts) {
                // remap to [-.5,.5]
                const auto uv = (v.fUV - SkPoint{0.5,0.5});
                const auto angle = kMaxRotate * t * uv.length();

                v.fPos = SkMatrix::RotateRad(angle).mapPoint(uv) + SkPoint{0.5, 0.5};
            }
        },
    },
    {
        "Cylinderator",
        // Simulate a cylinder rolling sideways across the 1x1 uv space.
        [](std::vector<ColorVertex>& verts, float t) {
            static constexpr float kCylRadius = .2f;

            const auto cyl_pos = t;

            for (auto& v : verts) {
                if (v.fUV.fX <= cyl_pos) {
                    v.fPos = v.fUV;
                    continue;
                }

                const auto arc_len = v.fUV.fX - cyl_pos,
                           arc_ang = arc_len/kCylRadius;

                v.fPos = SkPoint{
                    cyl_pos + std::sin(arc_ang)*kCylRadius,
                    v.fUV.fY,
                };
            }
        },
    },
    {
        "None",
        [](std::vector<ColorVertex>& verts, float) { for (auto& v : verts) { v.fPos = v.fUV; } },
    },
};

class MeshGradientSlide final : public Slide {
public:
    MeshGradientSlide()
        : fTimeMapper({0.5f, 0}, {0.5f, 1})
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

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SK_ColorWHITE);

        static constexpr float kMeshViewFract = 0.85f;
        const float mesh_size = std::min(fSize.fWidth, fSize.fHeight) * kMeshViewFract;

        canvas->translate((fSize.fWidth  - mesh_size) * 0.5f,
                          (fSize.fHeight - mesh_size) * 0.5f);
        canvas->scale(mesh_size, mesh_size);

        SkPaint paint;
        paint.setAntiAlias(true);

        // TODO: render the actual mesh
        for (const auto& v : fVertices) {
            paint.setColor4f(v.fColor);
            canvas->drawCircle(v.fPos, 0.005f, paint);
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

            fCurrentAnimator->fAanimate(fVertices, fTimeMapper.computeYFromX(t));

            // TODO: mesh triangulation
        }

        return true;
    }

private:
    void updateMesh(size_t new_count) {
        const size_t current_count = fVertices.size();

        fVertices.resize(new_count);

        for (size_t i = current_count; i < new_count; ++i) {
            const SkPoint uv = { fRNG.nextF(), fRNG.nextF() };
            fVertices[i] = { uv, uv, SkColor4f{ fRNG.nextF(), fRNG.nextF(), fRNG.nextF(), 1.f }};
        }

        if (new_count < current_count) {
            // Reset the RNG state
            fRNG.setSeed(0);
            static constexpr size_t kRandsPerVertex = 5;
            for (size_t i = 0; i < new_count * kRandsPerVertex; ++i) { fRNG.nextF(); }
        }
    }

    void drawControls() {
        ImGui::Begin("Mesh Options");

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

        if (ImGui::SliderInt("Vertex Count", &fVertexCountTo, 3, 256)) {
            fVertexCountTimeBase = 0;
            fVertexCountFrom = fVertices.size();
        }

        ImGui::SliderFloat("Speed", &fAnimationSpeed, 0.25, 4, "%.2f");

        ImGui::End();
    }

    SkSize                   fSize;
    std::vector<ColorVertex> fVertices;
    SkRandom                 fRNG;

    // Animation state
    const SkCubicMap         fTimeMapper,
                             fVertedCountTimeMapper;
    double                   fTimeBase               = 0,
                             fVertexCountTimeBase    = 0;
    int                      fVertexCountFrom        = 0,
                             fVertexCountTo          = 64;

    // UI stuff
    const VertexAnimator*    fCurrentAnimator = &gVertexAnimators[0];
    float                    fAnimationSpeed  = 1.f;
};

}  // anonymous namespace

DEF_SLIDE(return new MeshGradientSlide{};)
