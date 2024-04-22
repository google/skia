/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RendererProvider_DEFINED
#define skgpu_graphite_RendererProvider_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkVertices.h"
#include "src/gpu/graphite/Renderer.h"

#include <vector>

namespace skgpu::graphite {

class Caps;
class StaticBufferManager;

#ifdef SK_ENABLE_VELLO_SHADERS
class VelloRenderer;
#endif

/**
 * Graphite defines a limited set of renderers in order to increase the likelihood of batching
 * across draw calls, and reducing the number of shader permutations required. These Renderers are
 * stateless singletons and remain alive for the life of the Context and its Recorders.
 *
 * Because Renderers are immutable and the defined Renderers are created at context initialization,
 * RendererProvider is trivially thread-safe.
 */
class RendererProvider {
public:
    static bool IsVelloRendererSupported(const Caps*);

    ~RendererProvider();

    // TODO: Add configuration options to disable "optimization" renderers in favor of the more
    // general case, or renderers that won't be used by the application. When that's added, these
    // functions could return null.

    // Path rendering for fills and strokes
    const Renderer* stencilTessellatedCurvesAndTris(SkPathFillType type) const {
        return &fStencilTessellatedCurves[(int) type];
    }
    const Renderer* stencilTessellatedWedges(SkPathFillType type) const {
        return &fStencilTessellatedWedges[(int) type];
    }
    const Renderer* convexTessellatedWedges() const { return &fConvexTessellatedWedges; }
    const Renderer* tessellatedStrokes() const { return &fTessellatedStrokes; }

    // Coverage mask rendering
    const Renderer* coverageMask() const { return &fCoverageMask; }

    // Atlas'ed text rendering
    const Renderer* bitmapText(bool useLCDText) const { return &fBitmapText[useLCDText]; }
    const Renderer* sdfText(bool useLCDText) const { return &fSDFText[useLCDText]; }

    // Mesh rendering
    const Renderer* vertices(SkVertices::VertexMode mode, bool hasColors, bool hasTexCoords) const {
        SkASSERT(mode != SkVertices::kTriangleFan_VertexMode); // Should be converted to kTriangles
        bool triStrip = mode == SkVertices::kTriangleStrip_VertexMode;
        return &fVertices[4*triStrip + 2*hasColors + hasTexCoords];
    }

    // Filled and stroked [r]rects
    const Renderer* analyticRRect() const { return &fAnalyticRRect; }

    // Per-edge AA quadrilaterals
    const Renderer* perEdgeAAQuad() const { return &fPerEdgeAAQuad; }

    const Renderer* analyticBlur() const { return &fAnalyticBlur; }

    // TODO: May need to add support for inverse filled strokes (need to check SVG spec if this is a
    // real thing).

    // Iterate over all available Renderers to combine with specified paint combinations when
    // pre-compiling pipelines.
    SkSpan<const Renderer* const> renderers() const {
        return {fRenderers.data(), fRenderers.size()};
    }

    const RenderStep* lookup(uint32_t uniqueID) const;

#ifdef SK_ENABLE_VELLO_SHADERS
    // Compute shader-based path renderer and compositor.
    const VelloRenderer* velloRenderer() const { return fVelloRenderer.get(); }
#endif

private:
    static constexpr int kPathTypeCount = 4;
    static constexpr int kVerticesCount = 8; // 2 modes * 2 color configs * 2 tex coord configs

    friend class Context; // for ctor

    // TODO: Take in caps that determines which Renderers to use for each category
    RendererProvider(const Caps*, StaticBufferManager* bufferManager);

    // Cannot be moved or copied
    RendererProvider(const RendererProvider&) = delete;
    RendererProvider(RendererProvider&&) = delete;

    // Renderers are composed of 1+ steps, and some steps can be shared by multiple Renderers.
    // Renderers don't keep their RenderSteps alive so RendererProvider holds them here.
    std::vector<std::unique_ptr<RenderStep>> fRenderSteps;

    // NOTE: Keep all Renderers dense to support automatically completing 'fRenderers'.
    Renderer fStencilTessellatedCurves[kPathTypeCount];
    Renderer fStencilTessellatedWedges[kPathTypeCount];
    Renderer fConvexTessellatedWedges;
    Renderer fTessellatedStrokes;

    Renderer fCoverageMask;

    Renderer fBitmapText[2];  // bool isLCD
    Renderer fSDFText[2]; // bool isLCD

    Renderer fAnalyticRRect;
    Renderer fPerEdgeAAQuad;

    Renderer fAnalyticBlur;

    Renderer fVertices[kVerticesCount];

    // Aggregate of all enabled Renderers for convenient iteration when pre-compiling
    std::vector<const Renderer*> fRenderers;

#ifdef SK_ENABLE_VELLO_SHADERS
    std::unique_ptr<VelloRenderer> fVelloRenderer;
#endif
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_RendererProvider_DEFINED
