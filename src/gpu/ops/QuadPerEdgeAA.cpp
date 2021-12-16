/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/QuadPerEdgeAA.h"

#include "include/private/SkVx.h"
#include "src/gpu/GrMeshDrawTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

static_assert((int)GrQuadAAFlags::kLeft   == SkCanvas::kLeft_QuadAAFlag);
static_assert((int)GrQuadAAFlags::kTop    == SkCanvas::kTop_QuadAAFlag);
static_assert((int)GrQuadAAFlags::kRight  == SkCanvas::kRight_QuadAAFlag);
static_assert((int)GrQuadAAFlags::kBottom == SkCanvas::kBottom_QuadAAFlag);
static_assert((int)GrQuadAAFlags::kNone   == SkCanvas::kNone_QuadAAFlags);
static_assert((int)GrQuadAAFlags::kAll    == SkCanvas::kAll_QuadAAFlags);

namespace skgpu::v1::QuadPerEdgeAA {

namespace {

using VertexSpec = skgpu::v1::QuadPerEdgeAA::VertexSpec;
using CoverageMode = skgpu::v1::QuadPerEdgeAA::CoverageMode;
using ColorType = skgpu::v1::QuadPerEdgeAA::ColorType;

// Generic WriteQuadProc that can handle any VertexSpec. It writes the 4 vertices in triangle strip
// order, although the data per-vertex is dependent on the VertexSpec.
void write_quad_generic(VertexWriter* vb,
                        const VertexSpec& spec,
                        const GrQuad* deviceQuad,
                        const GrQuad* localQuad,
                        const float coverage[4],
                        const SkPMColor4f& color,
                        const SkRect& geomSubset,
                        const SkRect& texSubset) {
    static constexpr auto If = VertexWriter::If<float>;

    SkASSERT(!spec.hasLocalCoords() || localQuad);

    CoverageMode mode = spec.coverageMode();
    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << If(spec.deviceQuadType() == GrQuad::Type::kPerspective, deviceQuad->w(i))
            << If(mode == CoverageMode::kWithPosition, coverage[i]);

        // save color
        if (spec.hasVertexColors()) {
            bool wide = spec.colorType() == ColorType::kFloat;
            *vb << VertexColor(color * (mode == CoverageMode::kWithColor ? coverage[i] : 1), wide);
        }

        // save local position
        if (spec.hasLocalCoords()) {
            *vb << localQuad->x(i)
                << localQuad->y(i)
                << If(spec.localQuadType() == GrQuad::Type::kPerspective, localQuad->w(i));
        }

        // save the geometry subset
        if (spec.requiresGeometrySubset()) {
            *vb << geomSubset;
        }

        // save the texture subset
        if (spec.hasSubset()) {
            *vb << texSubset;
        }
    }
}

// Specialized WriteQuadProcs for particular VertexSpecs that show up frequently (determined
// experimentally through recorded GMs, SKPs, and SVGs, as well as SkiaRenderer's usage patterns):

// 2D (XY), no explicit coverage, vertex color, no locals, no geometry subset, no texture subsetn
// This represents simple, solid color or shader, non-AA (or AA with cov. as alpha) rects.
void write_2d_color(VertexWriter* vb,
                    const VertexSpec& spec,
                    const GrQuad* deviceQuad,
                    const GrQuad* localQuad,
                    const float coverage[4],
                    const SkPMColor4f& color,
                    const SkRect& geomSubset,
                    const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(!spec.hasLocalCoords());
    SkASSERT(spec.coverageMode() == CoverageMode::kNone ||
             spec.coverageMode() == CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(!spec.hasSubset());
    // We don't assert that localQuad == nullptr, since it is possible for FillRectOp to
    // accumulate local coords conservatively (paint not trivial), and then after analysis realize
    // the processors don't need local coordinates.

    bool wide = spec.colorType() == ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == CoverageMode::kWithColor || coverage[i] == 1.f);
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << VertexColor(color * coverage[i], wide);
    }
}

// 2D (XY), no explicit coverage, UV locals, no color, no geometry subset, no texture subset
// This represents opaque, non AA, textured rects
void write_2d_uv(VertexWriter* vb,
                 const VertexSpec& spec,
                 const GrQuad* deviceQuad,
                 const GrQuad* localQuad,
                 const float coverage[4],
                 const SkPMColor4f& color,
                 const SkRect& geomSubset,
                 const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kNone);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(!spec.hasSubset());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << localQuad->x(i)
            << localQuad->y(i);
    }
}

// 2D (XY), no explicit coverage, UV locals, vertex color, no geometry or texture subsets
// This represents transparent, non AA (or AA with cov. as alpha), textured rects
void write_2d_color_uv(VertexWriter* vb,
                       const VertexSpec& spec,
                       const GrQuad* deviceQuad,
                       const GrQuad* localQuad,
                       const float coverage[4],
                       const SkPMColor4f& color,
                       const SkRect& geomSubset,
                       const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kNone ||
             spec.coverageMode() == CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(!spec.hasSubset());
    SkASSERT(localQuad);

    bool wide = spec.colorType() == ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == CoverageMode::kWithColor || coverage[i] == 1.f);
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << VertexColor(color * coverage[i], wide)
            << localQuad->x(i)
            << localQuad->y(i);
    }
}

// 2D (XY), explicit coverage, UV locals, no color, no geometry subset, no texture subset
// This represents opaque, AA, textured rects
void write_2d_cov_uv(VertexWriter* vb,
                     const VertexSpec& spec,
                     const GrQuad* deviceQuad,
                     const GrQuad* localQuad,
                     const float coverage[4],
                     const SkPMColor4f& color,
                     const SkRect& geomSubset,
                     const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kWithPosition);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(!spec.hasSubset());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << coverage[i]
            << localQuad->x(i)
            << localQuad->y(i);
    }
}

// NOTE: The three _strict specializations below match the non-strict uv functions above, except
// that they also write the UV subset. These are included to benefit SkiaRenderer, which must make
// use of both fast and strict constrained subsets. When testing _strict was not that common across
// GMS, SKPs, and SVGs but we have little visibility into actual SkiaRenderer statistics. If
// SkiaRenderer can avoid subsets more, these 3 functions should probably be removed for simplicity.

// 2D (XY), no explicit coverage, UV locals, no color, tex subset but no geometry subset
// This represents opaque, non AA, textured rects with strict uv sampling
void write_2d_uv_strict(VertexWriter* vb,
                        const VertexSpec& spec,
                        const GrQuad* deviceQuad,
                        const GrQuad* localQuad,
                        const float coverage[4],
                        const SkPMColor4f& color,
                        const SkRect& geomSubset,
                        const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kNone);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(spec.hasSubset());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << localQuad->x(i)
            << localQuad->y(i)
            << texSubset;
    }
}

// 2D (XY), no explicit coverage, UV locals, vertex color, tex subset but no geometry subset
// This represents transparent, non AA (or AA with cov. as alpha), textured rects with strict sample
void write_2d_color_uv_strict(VertexWriter* vb,
                              const VertexSpec& spec,
                              const GrQuad* deviceQuad,
                              const GrQuad* localQuad,
                              const float coverage[4],
                              const SkPMColor4f& color,
                              const SkRect& geomSubset,
                              const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kNone ||
             spec.coverageMode() == CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(spec.hasSubset());
    SkASSERT(localQuad);

    bool wide = spec.colorType() == ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == CoverageMode::kWithColor || coverage[i] == 1.f);
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << VertexColor(color * coverage[i], wide)
            << localQuad->x(i)
            << localQuad->y(i)
            << texSubset;
    }
}

// 2D (XY), explicit coverage, UV locals, no color, tex subset but no geometry subset
// This represents opaque, AA, textured rects with strict uv sampling
void write_2d_cov_uv_strict(VertexWriter* vb,
                            const VertexSpec& spec,
                            const GrQuad* deviceQuad,
                            const GrQuad* localQuad,
                            const float coverage[4],
                            const SkPMColor4f& color,
                            const SkRect& geomSubset,
                            const SkRect& texSubset) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == CoverageMode::kWithPosition);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometrySubset());
    SkASSERT(spec.hasSubset());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        *vb << deviceQuad->x(i)
            << deviceQuad->y(i)
            << coverage[i]
            << localQuad->x(i)
            << localQuad->y(i)
            << texSubset;
    }
}

} // anonymous namespace

IndexBufferOption CalcIndexBufferOption(GrAAType aa, int numQuads) {
    if (aa == GrAAType::kCoverage) {
        return IndexBufferOption::kPictureFramed;
    } else if (numQuads > 1) {
        return IndexBufferOption::kIndexedRects;
    } else {
        return IndexBufferOption::kTriStrips;
    }
}

// This is a more elaborate version of fitsInBytes() that allows "no color" for white
ColorType MinColorType(SkPMColor4f color) {
    if (color == SK_PMColor4fWHITE) {
        return ColorType::kNone;
    } else {
        return color.fitsInBytes() ? ColorType::kByte : ColorType::kFloat;
    }
}

////////////////// Tessellator Implementation

Tessellator::WriteQuadProc Tessellator::GetWriteQuadProc(const VertexSpec& spec) {
    // All specialized writing functions requires 2D geometry and no geometry subset. This is not
    // the same as just checking device type vs. kRectilinear since non-AA general 2D quads do not
    // require a geometry subset and could then go through a fast path.
    if (spec.deviceQuadType() != GrQuad::Type::kPerspective && !spec.requiresGeometrySubset()) {
        CoverageMode mode = spec.coverageMode();
        if (spec.hasVertexColors()) {
            if (mode != CoverageMode::kWithPosition) {
                // Vertex colors, but no explicit coverage
                if (!spec.hasLocalCoords()) {
                    // Non-UV with vertex colors (possibly with coverage folded into alpha)
                    return write_2d_color;
                } else if (spec.localQuadType() != GrQuad::Type::kPerspective) {
                    // UV locals with vertex colors (possibly with coverage-as-alpha)
                    return spec.hasSubset() ? write_2d_color_uv_strict : write_2d_color_uv;
                }
            }
            // Else fall through; this is a spec that requires vertex colors and explicit coverage,
            // which means it's anti-aliased and the FPs don't support coverage as alpha, or
            // it uses 3D local coordinates.
        } else if (spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective) {
            if (mode == CoverageMode::kWithPosition) {
                // UV locals with explicit coverage
                return spec.hasSubset() ? write_2d_cov_uv_strict : write_2d_cov_uv;
            } else {
                SkASSERT(mode == CoverageMode::kNone);
                return spec.hasSubset() ? write_2d_uv_strict : write_2d_uv;
            }
        }
        // Else fall through to generic vertex function; this is a spec that has no vertex colors
        // and [no|uvr] local coords, which doesn't happen often enough to warrant specialization.
    }

    // Arbitrary spec hits the slow path
    return write_quad_generic;
}

Tessellator::Tessellator(const VertexSpec& spec, char* vertices)
        : fVertexSpec(spec)
        , fVertexWriter{vertices}
        , fWriteProc(Tessellator::GetWriteQuadProc(spec)) {}

void Tessellator::append(GrQuad* deviceQuad, GrQuad* localQuad,
                         const SkPMColor4f& color, const SkRect& uvSubset, GrQuadAAFlags aaFlags) {
    // We allow Tessellator to be created with a null vertices pointer for convenience, but it is
    // assumed it will never actually be used in those cases.
    SkASSERT(fVertexWriter);
    SkASSERT(deviceQuad->quadType() <= fVertexSpec.deviceQuadType());
    SkASSERT(localQuad || !fVertexSpec.hasLocalCoords());
    SkASSERT(!fVertexSpec.hasLocalCoords() || localQuad->quadType() <= fVertexSpec.localQuadType());

    static const float kFullCoverage[4] = {1.f, 1.f, 1.f, 1.f};
    static const float kZeroCoverage[4] = {0.f, 0.f, 0.f, 0.f};
    static const SkRect kIgnoredSubset = SkRect::MakeEmpty();

    if (fVertexSpec.usesCoverageAA()) {
        SkASSERT(fVertexSpec.coverageMode() == CoverageMode::kWithColor ||
                 fVertexSpec.coverageMode() == CoverageMode::kWithPosition);
        // Must calculate inner and outer quadrilaterals for the vertex coverage ramps, and possibly
        // a geometry subset if corners are not right angles
        SkRect geomSubset;
        if (fVertexSpec.requiresGeometrySubset()) {
#ifdef SK_USE_LEGACY_AA_QUAD_SUBSET
            geomSubset = deviceQuad->bounds();
            geomSubset.outset(0.5f, 0.5f); // account for AA expansion
#else
            // Our GP code expects a 0.5 outset rect (coverage is computed as 0 at the values of
            // the uniform). However, if we have quad edges that aren't supposed to be antialiased
            // they may lie close to the bounds. So in that case we outset by an additional 0.5.
            // This is a sort of backup clipping mechanism for cases where quad outsetting of nearly
            // parallel edges produces long thin extrusions from the original geometry.
            float outset = aaFlags == GrQuadAAFlags::kAll ? 0.5f : 1.f;
            geomSubset = deviceQuad->bounds().makeOutset(outset, outset);
#endif
        }

        if (aaFlags == GrQuadAAFlags::kNone) {
            // Have to write the coverage AA vertex structure, but there's no math to be done for a
            // non-aa quad batched into a coverage AA op.
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kFullCoverage, color,
                       geomSubset, uvSubset);
            // Since we pass the same corners in, the outer vertex structure will have 0 area and
            // the coverage interpolation from 1 to 0 will not be visible.
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kZeroCoverage, color,
                       geomSubset, uvSubset);
        } else {
            // Reset the tessellation helper to match the current geometry
            fAAHelper.reset(*deviceQuad, localQuad);

            // Edge inset/outset distance ordered LBTR, set to 0.5 for a half pixel if the AA flag
            // is turned on, or 0.0 if the edge is not anti-aliased.
            skvx::Vec<4, float> edgeDistances;
            if (aaFlags == GrQuadAAFlags::kAll) {
                edgeDistances = 0.5f;
            } else {
                edgeDistances = { (aaFlags & GrQuadAAFlags::kLeft)   ? 0.5f : 0.f,
                                  (aaFlags & GrQuadAAFlags::kBottom) ? 0.5f : 0.f,
                                  (aaFlags & GrQuadAAFlags::kTop)    ? 0.5f : 0.f,
                                  (aaFlags & GrQuadAAFlags::kRight)  ? 0.5f : 0.f };
            }

            // Write inner vertices first
            float coverage[4];
            fAAHelper.inset(edgeDistances, deviceQuad, localQuad).store(coverage);
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, coverage, color,
                       geomSubset, uvSubset);

            // Then outer vertices, which use 0.f for their coverage. If the inset was degenerate
            // to a line (had all coverages < 1), tweak the outset distance so the outer frame's
            // narrow axis reaches out to 2px, which gives better animation under translation.
            const bool hairline = aaFlags == GrQuadAAFlags::kAll &&
                                  coverage[0] < 1.f &&
                                  coverage[1] < 1.f &&
                                  coverage[2] < 1.f &&
                                  coverage[3] < 1.f;
            if (hairline) {
                skvx::Vec<4, float> len = fAAHelper.getEdgeLengths();
                // Using max guards us against trying to scale a degenerate triangle edge of 0 len
                // up to 2px. The shuffles are so that edge 0's adjustment is based on the lengths
                // of its connecting edges (1 and 2), and so forth.
                skvx::Vec<4, float> maxWH = max(skvx::shuffle<1, 0, 3, 2>(len),
                                                skvx::shuffle<2, 3, 0, 1>(len));
                // wh + 2e' = 2, so e' = (2 - wh) / 2 => e' = e * (2 - wh). But if w or h > 1, then
                // 2 - wh < 1 and represents the non-narrow axis so clamp to 1.
                edgeDistances *= max(1.f, 2.f - maxWH);
            }
            fAAHelper.outset(edgeDistances, deviceQuad, localQuad);
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kZeroCoverage, color,
                       geomSubset, uvSubset);
        }
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(fVertexSpec.coverageMode() == CoverageMode::kNone &&
                 !fVertexSpec.requiresGeometrySubset());
        fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kFullCoverage, color,
                   kIgnoredSubset, uvSubset);
    }
}

sk_sp<const GrBuffer> GetIndexBuffer(GrMeshDrawTarget* target,
                                     IndexBufferOption indexBufferOption) {
    auto resourceProvider = target->resourceProvider();

    switch (indexBufferOption) {
        case IndexBufferOption::kPictureFramed: return resourceProvider->refAAQuadIndexBuffer();
        case IndexBufferOption::kIndexedRects:  return resourceProvider->refNonAAQuadIndexBuffer();
        case IndexBufferOption::kTriStrips:     // fall through
        default:                                return nullptr;
    }
}

int QuadLimit(IndexBufferOption option) {
    switch (option) {
        case IndexBufferOption::kPictureFramed: return GrResourceProvider::MaxNumAAQuads();
        case IndexBufferOption::kIndexedRects:  return GrResourceProvider::MaxNumNonAAQuads();
        case IndexBufferOption::kTriStrips:     return SK_MaxS32; // not limited by an indexBuffer
    }

    SkUNREACHABLE;
}

void IssueDraw(const GrCaps& caps, GrOpsRenderPass* renderPass, const VertexSpec& spec,
               int runningQuadCount, int quadsInDraw, int maxVerts, int absVertBufferOffset) {
    if (spec.indexBufferOption() == IndexBufferOption::kTriStrips) {
        int offset = absVertBufferOffset +
                                    runningQuadCount * GrResourceProvider::NumVertsPerNonAAQuad();
        renderPass->draw(4, offset);
        return;
    }

    SkASSERT(spec.indexBufferOption() == IndexBufferOption::kPictureFramed ||
             spec.indexBufferOption() == IndexBufferOption::kIndexedRects);

    int maxNumQuads, numIndicesPerQuad, numVertsPerQuad;

    if (spec.indexBufferOption() == IndexBufferOption::kPictureFramed) {
        // AA uses 8 vertices and 30 indices per quad, basically nested rectangles
        maxNumQuads = GrResourceProvider::MaxNumAAQuads();
        numIndicesPerQuad = GrResourceProvider::NumIndicesPerAAQuad();
        numVertsPerQuad = GrResourceProvider::NumVertsPerAAQuad();
    } else {
        // Non-AA uses 4 vertices and 6 indices per quad
        maxNumQuads = GrResourceProvider::MaxNumNonAAQuads();
        numIndicesPerQuad = GrResourceProvider::NumIndicesPerNonAAQuad();
        numVertsPerQuad = GrResourceProvider::NumVertsPerNonAAQuad();
    }

    SkASSERT(runningQuadCount + quadsInDraw <= maxNumQuads);

    if (caps.avoidLargeIndexBufferDraws()) {
        // When we need to avoid large index buffer draws we modify the base vertex of the draw
        // which, in GL, requires rebinding all vertex attrib arrays, so a base index is generally
        // preferred.
        int offset = absVertBufferOffset + runningQuadCount * numVertsPerQuad;

        renderPass->drawIndexPattern(numIndicesPerQuad, quadsInDraw, maxNumQuads, numVertsPerQuad,
                                     offset);
    } else {
        int baseIndex = runningQuadCount * numIndicesPerQuad;
        int numIndicesToDraw = quadsInDraw * numIndicesPerQuad;

        int minVertex = runningQuadCount * numVertsPerQuad;
        int maxVertex = (runningQuadCount + quadsInDraw) * numVertsPerQuad - 1; // inclusive

        renderPass->drawIndexed(numIndicesToDraw, baseIndex, minVertex, maxVertex,
                                absVertBufferOffset);
    }
}

////////////////// VertexSpec Implementation

int VertexSpec::deviceDimensionality() const {
    return this->deviceQuadType() == GrQuad::Type::kPerspective ? 3 : 2;
}

int VertexSpec::localDimensionality() const {
    return fHasLocalCoords ? (this->localQuadType() == GrQuad::Type::kPerspective ? 3 : 2) : 0;
}

CoverageMode VertexSpec::coverageMode() const {
    if (this->usesCoverageAA()) {
        if (this->compatibleWithCoverageAsAlpha() && this->hasVertexColors() &&
            !this->requiresGeometrySubset()) {
            // Using a geometric subset acts as a second source of coverage and folding
            // the original coverage into color makes it impossible to apply the color's
            // alpha to the geometric subset's coverage when the original shape is clipped.
            return CoverageMode::kWithColor;
        } else {
            return CoverageMode::kWithPosition;
        }
    } else {
        return CoverageMode::kNone;
    }
}

// This needs to stay in sync w/ QuadPerEdgeAAGeometryProcessor::initializeAttrs
size_t VertexSpec::vertexSize() const {
    bool needsPerspective = (this->deviceDimensionality() == 3);
    CoverageMode coverageMode = this->coverageMode();

    size_t count = 0;

    if (coverageMode == CoverageMode::kWithPosition) {
        if (needsPerspective) {
            count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
        } else {
            count += GrVertexAttribTypeSize(kFloat2_GrVertexAttribType) +
                     GrVertexAttribTypeSize(kFloat_GrVertexAttribType);
        }
    } else {
        if (needsPerspective) {
            count += GrVertexAttribTypeSize(kFloat3_GrVertexAttribType);
        } else {
            count += GrVertexAttribTypeSize(kFloat2_GrVertexAttribType);
        }
    }

    if (this->requiresGeometrySubset()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    count += this->localDimensionality() * GrVertexAttribTypeSize(kFloat_GrVertexAttribType);

    if (ColorType::kByte == this->colorType()) {
        count += GrVertexAttribTypeSize(kUByte4_norm_GrVertexAttribType);
    } else if (ColorType::kFloat == this->colorType()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    if (this->hasSubset()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    return count;
}

////////////////// Geometry Processor Implementation

class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const VertexSpec& spec) {
        return arena->make([&](void* ptr) {
            return new (ptr) QuadPerEdgeAAGeometryProcessor(spec);
        });
    }

    static GrGeometryProcessor* Make(SkArenaAlloc* arena,
                                     const VertexSpec& vertexSpec,
                                     const GrShaderCaps& caps,
                                     const GrBackendFormat& backendFormat,
                                     GrSamplerState samplerState,
                                     const GrSwizzle& swizzle,
                                     sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                     Saturate saturate) {
        return arena->make([&](void* ptr) {
            return new (ptr) QuadPerEdgeAAGeometryProcessor(
                    vertexSpec, caps, backendFormat, samplerState, swizzle,
                    std::move(textureColorSpaceXform), saturate);
        });
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void addToKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // texturing, device-dimensions are single bit flags
        b->addBool(fTexSubset.isInitialized(),    "subset");
        b->addBool(fSampler.isInitialized(),      "textured");
        b->addBool(fNeedsPerspective,             "perspective");
        b->addBool((fSaturate == Saturate::kYes), "saturate");

        b->addBool(fLocalCoord.isInitialized(),   "hasLocalCoords");
        if (fLocalCoord.isInitialized()) {
            // 2D (0) or 3D (1)
            b->addBits(1, (kFloat3_GrVertexAttribType == fLocalCoord.cpuType()), "localCoordsType");
        }
        b->addBool(fColor.isInitialized(),        "hasColor");
        if (fColor.isInitialized()) {
            // bytes (0) or floats (1)
            b->addBits(1, (kFloat4_GrVertexAttribType == fColor.cpuType()), "colorType");
        }
        // and coverage mode, 00 for none, 01 for withposition, 10 for withcolor, 11 for
        // position+geomsubset
        uint32_t coverageKey = 0;
        SkASSERT(!fGeomSubset.isInitialized() || fCoverageMode == CoverageMode::kWithPosition);
        if (fCoverageMode != CoverageMode::kNone) {
            coverageKey = fGeomSubset.isInitialized()
                                  ? 0x3
                                  : (CoverageMode::kWithPosition == fCoverageMode ? 0x1 : 0x2);
        }
        b->addBits(2, coverageKey, "coverageMode");

        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()), "colorSpaceXform");
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override {
        class Impl : public ProgramImpl {
        public:
            void setData(const GrGLSLProgramDataManager& pdman,
                         const GrShaderCaps&,
                         const GrGeometryProcessor& geomProc) override {
                const auto& gp = geomProc.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.setData(pdman, gp.fTextureColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;

                const auto& gp = args.fGeomProc.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                       gp.fTextureColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(gp);

                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    // Strip last channel from the vertex attribute to remove coverage and get the
                    // actual position
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("float3 position = %s.xyz;",
                                                       gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("float2 position = %s.xy;",
                                                       gp.fPosition.name());
                    }
                    gpArgs->fPositionVar = {"position",
                                            gp.fNeedsPerspective ? kFloat3_GrSLType
                                                                 : kFloat2_GrSLType,
                                            GrShaderVar::TypeModifier::None};
                } else {
                    // No coverage to eliminate
                    gpArgs->fPositionVar = gp.fPosition.asShaderVar();
                }

                // This attribute will be uninitialized if earlier FP analysis determined no
                // local coordinates are needed (and this will not include the inline texture
                // fetch this GP does before invoking FPs).
                gpArgs->fLocalCoordVar = gp.fLocalCoord.asShaderVar();

                // Solid color before any texturing gets modulated in
                const char* blendDst;
                if (gp.fColor.isInitialized()) {
                    SkASSERT(gp.fCoverageMode != CoverageMode::kWithColor || !gp.fNeedsPerspective);
                    // The color cannot be flat if the varying coverage has been modulated into it
                    args.fFragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
                    args.fVaryingHandler->addPassThroughAttribute(
                            gp.fColor.asShaderVar(),
                            args.fOutputColor,
                            gp.fCoverageMode == CoverageMode::kWithColor
                                    ? Interpolation::kInterpolated
                                    : Interpolation::kCanBeFlat);
                    blendDst = args.fOutputColor;
                } else {
                    // Output color must be initialized to something
                    args.fFragBuilder->codeAppendf("half4 %s = half4(1);", args.fOutputColor);
                    blendDst = nullptr;
                }

                // If there is a texture, must also handle texture coordinates and reading from
                // the texture in the fragment shader before continuing to fragment processors.
                if (gp.fSampler.isInitialized()) {
                    // Texture coordinates clamped by the subset on the fragment shader; if the GP
                    // has a texture, it's guaranteed to have local coordinates
                    args.fFragBuilder->codeAppend("float2 texCoord;");
                    if (gp.fLocalCoord.cpuType() == kFloat3_GrVertexAttribType) {
                        // Can't do a pass through since we need to perform perspective division
                        GrGLSLVarying v(gp.fLocalCoord.gpuType());
                        args.fVaryingHandler->addVarying(gp.fLocalCoord.name(), &v);
                        args.fVertBuilder->codeAppendf("%s = %s;",
                                                       v.vsOut(), gp.fLocalCoord.name());
                        args.fFragBuilder->codeAppendf("texCoord = %s.xy / %s.z;",
                                                       v.fsIn(), v.fsIn());
                    } else {
                        args.fVaryingHandler->addPassThroughAttribute(gp.fLocalCoord.asShaderVar(),
                                                                      "texCoord");
                    }

                    // Clamp the now 2D localCoordName variable by the subset if it is provided
                    if (gp.fTexSubset.isInitialized()) {
                        args.fFragBuilder->codeAppend("float4 subset;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fTexSubset.asShaderVar(),
                                                                      "subset",
                                                                      Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "texCoord = clamp(texCoord, subset.LT, subset.RB);");
                    }

                    // Now modulate the starting output color by the texture lookup
                    args.fFragBuilder->codeAppendf(
                            "%s = %s(",
                            args.fOutputColor,
                            (gp.fSaturate == Saturate::kYes) ? "saturate" : "");
                    args.fFragBuilder->appendTextureLookupAndBlend(
                            blendDst, SkBlendMode::kModulate, args.fTexSamplers[0],
                            "texCoord", &fTextureColorSpaceXformHelper);
                    args.fFragBuilder->codeAppend(");");
                } else {
                    // Saturate is only intended for use with a proxy to account for the fact
                    // that TextureOp skips SkPaint conversion, which normally handles this.
                    SkASSERT(gp.fSaturate == Saturate::kNo);
                }

                // And lastly, output the coverage calculation code
                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    GrGLSLVarying coverage(kFloat_GrSLType);
                    args.fVaryingHandler->addVarying("coverage", &coverage);
                    if (gp.fNeedsPerspective) {
                        // Multiply by "W" in the vertex shader, then by 1/w (sk_FragCoord.w) in
                        // the fragment shader to get screen-space linear coverage.
                        args.fVertBuilder->codeAppendf("%s = %s.w * %s.z;",
                                                       coverage.vsOut(), gp.fPosition.name(),
                                                       gp.fPosition.name());
                        args.fFragBuilder->codeAppendf("float coverage = %s * sk_FragCoord.w;",
                                                        coverage.fsIn());
                    } else {
                        args.fVertBuilder->codeAppendf("%s = %s;",
                                                       coverage.vsOut(), gp.fCoverage.name());
                        args.fFragBuilder->codeAppendf("float coverage = %s;", coverage.fsIn());
                    }

                    if (gp.fGeomSubset.isInitialized()) {
                        // Calculate distance from sk_FragCoord to the 4 edges of the subset
                        // and clamp them to (0, 1). Use the minimum of these and the original
                        // coverage. This only has to be done in the exterior triangles, the
                        // interior of the quad geometry can never be clipped by the subset box.
                        args.fFragBuilder->codeAppend("float4 geoSubset;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fGeomSubset.asShaderVar(),
                                                                      "geoSubset",
                                                                      Interpolation::kCanBeFlat);
#ifdef SK_USE_LEGACY_AA_QUAD_SUBSET
                        args.fFragBuilder->codeAppend(
                                "if (coverage < 0.5) {"
                                "   float4 dists4 = clamp(float4(1, 1, -1, -1) * "
                                        "(sk_FragCoord.xyxy - geoSubset), 0, 1);"
                                "   float2 dists2 = dists4.xy * dists4.zw;"
                                "   coverage = min(coverage, dists2.x * dists2.y);"
                                "}");
#else
                        args.fFragBuilder->codeAppend(
                                // This is lifted from GrAARectEffect. It'd be nice if we could
                                // invoke a FP from a GP rather than duplicate this code.
                                "half4 dists4 = clamp(half4(1, 1, -1, -1) * "
                                               "half4(sk_FragCoord.xyxy - geoSubset), 0, 1);\n"
                                "half2 dists2 = dists4.xy + dists4.zw - 1;\n"
                                "half subsetCoverage = dists2.x * dists2.y;\n"
                                "coverage = min(coverage, subsetCoverage);");
#endif
                    }

                    args.fFragBuilder->codeAppendf("half4 %s = half4(half(coverage));",
                                                   args.fOutputCoverage);
                } else {
                    // Set coverage to 1, since it's either non-AA or the coverage was already
                    // folded into the output color
                    SkASSERT(!gp.fGeomSubset.isInitialized());
                    args.fFragBuilder->codeAppendf("const half4 %s = half4(1);",
                                                   args.fOutputCoverage);
                }
            }

            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };

        return std::make_unique<Impl>();
    }

private:
    using Saturate = skgpu::v1::TextureOp::Saturate;

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(nullptr) {
        SkASSERT(!spec.hasSubset());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(0);
    }

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec,
                                   const GrShaderCaps& caps,
                                   const GrBackendFormat& backendFormat,
                                   GrSamplerState samplerState,
                                   const GrSwizzle& swizzle,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                   Saturate saturate)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fSaturate(saturate)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(samplerState, backendFormat, swizzle) {
        SkASSERT(spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    // This needs to stay in sync w/ VertexSpec::vertexSize
    void initializeAttrs(const VertexSpec& spec) {
        fNeedsPerspective = spec.deviceDimensionality() == 3;
        fCoverageMode = spec.coverageMode();

        if (fCoverageMode == CoverageMode::kWithPosition) {
            if (fNeedsPerspective) {
                fPosition = {"positionWithCoverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            } else {
                fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
                fCoverage = {"coverage", kFloat_GrVertexAttribType, kFloat_GrSLType};
            }
        } else {
            if (fNeedsPerspective) {
                fPosition = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            } else {
                fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
            }
        }

        // Need a geometry subset when the quads are AA and not rectilinear, since their AA
        // outsetting can go beyond a half pixel.
        if (spec.requiresGeometrySubset()) {
            fGeomSubset = {"geomSubset", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        int localDim = spec.localDimensionality();
        if (localDim == 3) {
            fLocalCoord = {"localCoord", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else if (localDim == 2) {
            fLocalCoord = {"localCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        } // else localDim == 0 and attribute remains uninitialized

        if (spec.hasVertexColors()) {
            fColor = MakeColorAttribute("color", ColorType::kFloat == spec.colorType());
        }

        if (spec.hasSubset()) {
            fTexSubset = {"texSubset", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        this->setVertexAttributes(&fPosition, 6);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition; // May contain coverage as last channel
    Attribute fCoverage; // Used for non-perspective position to avoid Intel Metal issues
    Attribute fColor; // May have coverage modulated in if the FPs support it
    Attribute fLocalCoord;
    Attribute fGeomSubset; // Screen-space bounding box on geometry+aa outset
    Attribute fTexSubset; // Texture-space bounding box on local coords

    // The positions attribute may have coverage built into it, so float3 is an ambiguous type
    // and may mean 2d with coverage, or 3d with no coverage
    bool fNeedsPerspective;
    // Should saturate() be called on the color? Only relevant when created with a texture.
    Saturate fSaturate = Saturate::kNo;
    CoverageMode fCoverageMode;

    // Color space will be null and fSampler.isInitialized() returns false when the GP is configured
    // to skip texturing.
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    TextureSampler fSampler;

    using INHERITED = GrGeometryProcessor;
};

GrGeometryProcessor* MakeProcessor(SkArenaAlloc* arena, const VertexSpec& spec) {
    return QuadPerEdgeAAGeometryProcessor::Make(arena, spec);
}

GrGeometryProcessor* MakeTexturedProcessor(SkArenaAlloc* arena,
                                           const VertexSpec& spec,
                                           const GrShaderCaps& caps,
                                           const GrBackendFormat& backendFormat,
                                           GrSamplerState samplerState,
                                           const GrSwizzle& swizzle,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                           Saturate saturate) {
    return QuadPerEdgeAAGeometryProcessor::Make(arena, spec, caps, backendFormat, samplerState,
                                                swizzle, std::move(textureColorSpaceXform),
                                                saturate);
}

} // namespace skgpu::v1::QuadPerEdgeAA
