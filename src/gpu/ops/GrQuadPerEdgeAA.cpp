/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrQuadPerEdgeAA.h"

#include "include/private/SkVx.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"


namespace {

// Generic WriteQuadProc that can handle any VertexSpec. It writes the 4 vertices in triangle strip
// order, although the data per-vertex is dependent on the VertexSpec.
static void write_quad_generic(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                               const GrQuad* deviceQuad, const GrQuad* localQuad,
                               const float coverage[4], const SkPMColor4f& color,
                               const SkRect& geomDomain, const SkRect& texDomain) {
    static constexpr auto If = GrVertexWriter::If<float>;

    SkASSERT(!spec.hasLocalCoords() || localQuad);

    GrQuadPerEdgeAA::CoverageMode mode = spec.coverageMode();
    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        vb->write(deviceQuad->x(i), deviceQuad->y(i),
                  If(spec.deviceQuadType() == GrQuad::Type::kPerspective, deviceQuad->w(i)),
                  If(mode == GrQuadPerEdgeAA::CoverageMode::kWithPosition, coverage[i]));

        // save color
        if (spec.hasVertexColors()) {
            bool wide = spec.colorType() == GrQuadPerEdgeAA::ColorType::kFloat;
            vb->write(GrVertexColor(
                color * (mode == GrQuadPerEdgeAA::CoverageMode::kWithColor ? coverage[i] : 1.f),
                wide));
        }

        // save local position
        if (spec.hasLocalCoords()) {
            vb->write(localQuad->x(i), localQuad->y(i),
                      If(spec.localQuadType() == GrQuad::Type::kPerspective, localQuad->w(i)));
        }

        // save the geometry domain
        if (spec.requiresGeometryDomain()) {
            vb->write(geomDomain);
        }

        // save the texture domain
        if (spec.hasDomain()) {
            vb->write(texDomain);
        }
    }
}

// Specialized WriteQuadProcs for particular VertexSpecs that show up frequently (determined
// experimentally through recorded GMs, SKPs, and SVGs, as well as SkiaRenderer's usage patterns):

// 2D (XY), no explicit coverage, vertex color, no locals, no geometry domain, no texture domain
// This represents simple, solid color or shader, non-AA (or AA with cov. as alpha) rects.
static void write_2d_color(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                           const GrQuad* deviceQuad, const GrQuad* localQuad,
                           const float coverage[4], const SkPMColor4f& color,
                           const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(!spec.hasLocalCoords());
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kNone ||
             spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(!spec.hasDomain());
    // We don't assert that localQuad == nullptr, since it is possible for GrFillRectOp to
    // accumulate local coords conservatively (paint not trivial), and then after analysis realize
    // the processors don't need local coordinates.

    bool wide = spec.colorType() == GrQuadPerEdgeAA::ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor ||
                 coverage[i] == 1.f);
        vb->write(deviceQuad->x(i), deviceQuad->y(i), GrVertexColor(color * coverage[i], wide));
    }
}

// 2D (XY), no explicit coverage, UV locals, no color, no geometry domain, no texture domain
// This represents opaque, non AA, textured rects
static void write_2d_uv(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                        const GrQuad* deviceQuad, const GrQuad* localQuad,
                        const float coverage[4], const SkPMColor4f& color,
                        const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kNone);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(!spec.hasDomain());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        vb->write(deviceQuad->x(i), deviceQuad->y(i), localQuad->x(i), localQuad->y(i));
    }
}

// 2D (XY), no explicit coverage, UV locals, vertex color, no geometry or texture domains
// This represents transparent, non AA (or AA with cov. as alpha), textured rects
static void write_2d_color_uv(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                              const GrQuad* deviceQuad, const GrQuad* localQuad,
                              const float coverage[4], const SkPMColor4f& color,
                              const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kNone ||
             spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(!spec.hasDomain());
    SkASSERT(localQuad);

    bool wide = spec.colorType() == GrQuadPerEdgeAA::ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor ||
                 coverage[i] == 1.f);
        vb->write(deviceQuad->x(i), deviceQuad->y(i), GrVertexColor(color * coverage[i], wide),
                  localQuad->x(i), localQuad->y(i));
    }
}

// 2D (XY), explicit coverage, UV locals, no color, no geometry domain, no texture domain
// This represents opaque, AA, textured rects
static void write_2d_cov_uv(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                            const GrQuad* deviceQuad, const GrQuad* localQuad,
                            const float coverage[4], const SkPMColor4f& color,
                            const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithPosition);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(!spec.hasDomain());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        vb->write(deviceQuad->x(i), deviceQuad->y(i), coverage[i],
                  localQuad->x(i), localQuad->y(i));
    }
}

// NOTE: The three _strict specializations below match the non-strict uv functions above, except
// that they also write the UV domain. These are included to benefit SkiaRenderer, which must make
// use of both fast and strict constrained domains. When testing _strict was not that common across
// GMS, SKPs, and SVGs but we have little visibility into actual SkiaRenderer statistics. If
// SkiaRenderer can avoid domains more, these 3 functions should probably be removed for simplicity.

// 2D (XY), no explicit coverage, UV locals, no color, tex domain but no geometry domain
// This represents opaque, non AA, textured rects with strict uv sampling
static void write_2d_uv_strict(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                               const GrQuad* deviceQuad, const GrQuad* localQuad,
                               const float coverage[4], const SkPMColor4f& color,
                               const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kNone);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(spec.hasDomain());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        vb->write(deviceQuad->x(i), deviceQuad->y(i), localQuad->x(i), localQuad->y(i), texDomain);
    }
}

// 2D (XY), no explicit coverage, UV locals, vertex color, tex domain but no geometry domain
// This represents transparent, non AA (or AA with cov. as alpha), textured rects with strict sample
static void write_2d_color_uv_strict(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                                     const GrQuad* deviceQuad, const GrQuad* localQuad,
                                     const float coverage[4], const SkPMColor4f& color,
                                     const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kNone ||
             spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor);
    SkASSERT(spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(spec.hasDomain());
    SkASSERT(localQuad);

    bool wide = spec.colorType() == GrQuadPerEdgeAA::ColorType::kFloat;
    for (int i = 0; i < 4; ++i) {
        // If this is not coverage-with-alpha, make sure coverage == 1 so it doesn't do anything
        SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithColor ||
                 coverage[i] == 1.f);
        vb->write(deviceQuad->x(i), deviceQuad->y(i), GrVertexColor(color * coverage[i], wide),
                  localQuad->x(i), localQuad->y(i), texDomain);
    }
}

// 2D (XY), explicit coverage, UV locals, no color, tex domain but no geometry domain
// This represents opaque, AA, textured rects with strict uv sampling
static void write_2d_cov_uv_strict(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                                   const GrQuad* deviceQuad, const GrQuad* localQuad,
                                   const float coverage[4], const SkPMColor4f& color,
                                   const SkRect& geomDomain, const SkRect& texDomain) {
    // Assert assumptions about VertexSpec
    SkASSERT(spec.deviceQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective);
    SkASSERT(spec.coverageMode() == GrQuadPerEdgeAA::CoverageMode::kWithPosition);
    SkASSERT(!spec.hasVertexColors());
    SkASSERT(!spec.requiresGeometryDomain());
    SkASSERT(spec.hasDomain());
    SkASSERT(localQuad);

    for (int i = 0; i < 4; ++i) {
        vb->write(deviceQuad->x(i), deviceQuad->y(i), coverage[i],
                  localQuad->x(i), localQuad->y(i), texDomain);
    }
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

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
    // All specialized writing functions requires 2D geometry and no geometry domain. This is not
    // the same as just checking device type vs. kRectilinear since non-AA general 2D quads do not
    // require a geometry domain and could then go through a fast path.
    if (spec.deviceQuadType() != GrQuad::Type::kPerspective && !spec.requiresGeometryDomain()) {
        CoverageMode mode = spec.coverageMode();
        if (spec.hasVertexColors()) {
            if (mode != CoverageMode::kWithPosition) {
                // Vertex colors, but no explicit coverage
                if (!spec.hasLocalCoords()) {
                    // Non-UV with vertex colors (possibly with coverage folded into alpha)
                    return write_2d_color;
                } else if (spec.localQuadType() != GrQuad::Type::kPerspective) {
                    // UV locals with vertex colors (possibly with coverage-as-alpha)
                    return spec.hasDomain() ? write_2d_color_uv_strict : write_2d_color_uv;
                }
            }
            // Else fall through; this is a spec that requires vertex colors and explicit coverage,
            // which means it's anti-aliased and the FPs don't support coverage as alpha, or
            // it uses 3D local coordinates.
        } else if (spec.hasLocalCoords() && spec.localQuadType() != GrQuad::Type::kPerspective) {
            if (mode == CoverageMode::kWithPosition) {
                // UV locals with explicit coverage
                return spec.hasDomain() ? write_2d_cov_uv_strict : write_2d_cov_uv;
            } else {
                SkASSERT(mode == CoverageMode::kNone);
                return spec.hasDomain() ? write_2d_uv_strict : write_2d_uv;
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
                         const SkPMColor4f& color, const SkRect& uvDomain, GrQuadAAFlags aaFlags) {
    // We allow Tessellator to be created with a null vertices pointer for convenience, but it is
    // assumed it will never actually be used in those cases.
    SkASSERT(fVertexWriter.fPtr);
    SkASSERT(deviceQuad->quadType() <= fVertexSpec.deviceQuadType());
    SkASSERT(localQuad || !fVertexSpec.hasLocalCoords());
    SkASSERT(!fVertexSpec.hasLocalCoords() || localQuad->quadType() <= fVertexSpec.localQuadType());

    static const float kFullCoverage[4] = {1.f, 1.f, 1.f, 1.f};
    static const float kZeroCoverage[4] = {0.f, 0.f, 0.f, 0.f};
    static const SkRect kIgnoredDomain = SkRect::MakeEmpty();

    if (fVertexSpec.usesCoverageAA()) {
        SkASSERT(fVertexSpec.coverageMode() == CoverageMode::kWithColor ||
                 fVertexSpec.coverageMode() == CoverageMode::kWithPosition);
        // Must calculate inner and outer quadrilaterals for the vertex coverage ramps, and possibly
        // a geometry domain if corners are not right angles
        SkRect geomDomain;
        if (fVertexSpec.requiresGeometryDomain()) {
            geomDomain = deviceQuad->bounds();
            geomDomain.outset(0.5f, 0.5f); // account for AA expansion
        }

        if (aaFlags == GrQuadAAFlags::kNone) {
            // Have to write the coverage AA vertex structure, but there's no math to be done for a
            // non-aa quad batched into a coverage AA op.
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kFullCoverage, color,
                       geomDomain, uvDomain);
            // Since we pass the same corners in, the outer vertex structure will have 0 area and
            // the coverage interpolation from 1 to 0 will not be visible.
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kZeroCoverage, color,
                       geomDomain, uvDomain);
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
                       geomDomain, uvDomain);

            // Then outer vertices, which use 0.f for their coverage
            fAAHelper.outset(edgeDistances, deviceQuad, localQuad);
            fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kZeroCoverage, color,
                       geomDomain, uvDomain);
        }
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(fVertexSpec.coverageMode() == CoverageMode::kNone &&
                 !fVertexSpec.requiresGeometryDomain());
        fWriteProc(&fVertexWriter, fVertexSpec, deviceQuad, localQuad, kFullCoverage, color,
                   kIgnoredDomain, uvDomain);
    }
}

sk_sp<const GrBuffer> GetIndexBuffer(GrMeshDrawOp::Target* target,
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

void ConfigureMesh(const GrCaps& caps, GrMesh* mesh, const VertexSpec& spec,
                   int runningQuadCount, int quadsInDraw, int maxVerts,
                   sk_sp<const GrBuffer> vertexBuffer,
                   sk_sp<const GrBuffer> indexBuffer, int absVertBufferOffset) {
    SkASSERT(vertexBuffer);

    mesh->setPrimitiveType(spec.primitiveType());

    if (spec.indexBufferOption() == IndexBufferOption::kTriStrips) {
        SkASSERT(!indexBuffer);

        mesh->setNonIndexedNonInstanced(4);
        int offset = absVertBufferOffset +
                                    runningQuadCount * GrResourceProvider::NumVertsPerNonAAQuad();
        mesh->setVertexData(std::move(vertexBuffer), offset);
        return;
    }

    SkASSERT(spec.indexBufferOption() == IndexBufferOption::kPictureFramed ||
             spec.indexBufferOption() == IndexBufferOption::kIndexedRects);
    SkASSERT(indexBuffer);

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

        mesh->setIndexedPatterned(std::move(indexBuffer), numIndicesPerQuad,
                                  numVertsPerQuad, quadsInDraw, maxNumQuads);
        mesh->setVertexData(std::move(vertexBuffer), offset);
    } else {
        int baseIndex = runningQuadCount * numIndicesPerQuad;
        int numIndicesToDraw = quadsInDraw * numIndicesPerQuad;

        int minVertex = runningQuadCount * numVertsPerQuad;
        int maxVertex = (runningQuadCount + quadsInDraw) * numVertsPerQuad;

        mesh->setIndexed(std::move(indexBuffer), numIndicesToDraw,
                         baseIndex, minVertex, maxVertex, GrPrimitiveRestart::kNo);
        mesh->setVertexData(std::move(vertexBuffer), absVertBufferOffset);
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
            !this->requiresGeometryDomain()) {
            // Using a geometric domain acts as a second source of coverage and folding
            // the original coverage into color makes it impossible to apply the color's
            // alpha to the geometric domain's coverage when the original shape is clipped.
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

    if (this->requiresGeometryDomain()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    count += this->localDimensionality() * GrVertexAttribTypeSize(kFloat_GrVertexAttribType);

    if (ColorType::kByte == this->colorType()) {
        count += GrVertexAttribTypeSize(kUByte4_norm_GrVertexAttribType);
    } else if (ColorType::kFloat == this->colorType()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    if (this->hasDomain()) {
        count += GrVertexAttribTypeSize(kFloat4_GrVertexAttribType);
    }

    return count;
}

////////////////// Geometry Processor Implementation

class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:
    using Saturate = GrTextureOp::Saturate;

    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const VertexSpec& spec) {
        return arena->make<QuadPerEdgeAAGeometryProcessor>(spec);
    }

    static GrGeometryProcessor* Make(SkArenaAlloc* arena, const VertexSpec& vertexSpec,
                                     const GrShaderCaps& caps,
                                     const GrBackendFormat& backendFormat,
                                     const GrSamplerState& samplerState,
                                     const GrSwizzle& swizzle,
                                     sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                     Saturate saturate) {
        return arena->make<QuadPerEdgeAAGeometryProcessor>(
                vertexSpec, caps, backendFormat, samplerState, swizzle,
                std::move(textureColorSpaceXform), saturate);
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // texturing, device-dimensions are single bit flags
        uint32_t x = (fTexDomain.isInitialized() ? 0 : 0x1)
                   | (fSampler.isInitialized()   ? 0 : 0x2)
                   | (fNeedsPerspective          ? 0 : 0x4)
                   | (fSaturate == Saturate::kNo ? 0 : 0x8);
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 0x10 : 0x20;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 0x40 : 0x80;
        }
        // and coverage mode, 00 for none, 01 for withposition, 10 for withcolor, 11 for
        // position+geomdomain
        SkASSERT(!fGeomDomain.isInitialized() || fCoverageMode == CoverageMode::kWithPosition);
        if (fCoverageMode != CoverageMode::kNone) {
            x |= fGeomDomain.isInitialized()
                         ? 0x300
                         : (CoverageMode::kWithPosition == fCoverageMode ? 0x100 : 0x200);
        }

        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(x);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         const CoordTransformRange& transformRange) override {
                const auto& gp = proc.cast<QuadPerEdgeAAGeometryProcessor>();
                if (gp.fLocalCoord.isInitialized()) {
                    this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
                }
                fTextureColorSpaceXformHelper.setData(pdman, gp.fTextureColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;

                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
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
                                            GrShaderVar::kNone_TypeModifier};
                } else {
                    // No coverage to eliminate
                    gpArgs->fPositionVar = gp.fPosition.asShaderVar();
                }

                // Handle local coordinates if they exist
                if (gp.fLocalCoord.isInitialized()) {
                    // NOTE: If the only usage of local coordinates is for the inline texture fetch
                    // before FPs, then there are no registered FPCoordTransforms and this ends up
                    // emitting nothing, so there isn't a duplication of local coordinates
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fLocalCoord.asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                // Solid color before any texturing gets modulated in
                if (gp.fColor.isInitialized()) {
                    SkASSERT(gp.fCoverageMode != CoverageMode::kWithColor || !gp.fNeedsPerspective);
                    // The color cannot be flat if the varying coverage has been modulated into it
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor,
                            gp.fCoverageMode == CoverageMode::kWithColor ?
                            Interpolation::kInterpolated : Interpolation::kCanBeFlat);
                } else {
                    // Output color must be initialized to something
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
                }

                // If there is a texture, must also handle texture coordinates and reading from
                // the texture in the fragment shader before continuing to fragment processors.
                if (gp.fSampler.isInitialized()) {
                    // Texture coordinates clamped by the domain on the fragment shader; if the GP
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
                        args.fVaryingHandler->addPassThroughAttribute(gp.fLocalCoord, "texCoord");
                    }

                    // Clamp the now 2D localCoordName variable by the domain if it is provided
                    if (gp.fTexDomain.isInitialized()) {
                        args.fFragBuilder->codeAppend("float4 domain;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fTexDomain, "domain",
                                                                      Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "texCoord = clamp(texCoord, domain.xy, domain.zw);");
                    }

                    // Now modulate the starting output color by the texture lookup
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                    args.fFragBuilder->codeAppend(";");
                    if (gp.fSaturate == Saturate::kYes) {
                        args.fFragBuilder->codeAppendf("%s = saturate(%s);",
                                                       args.fOutputColor, args.fOutputColor);
                    }
                } else {
                    // Saturate is only intended for use with a proxy to account for the fact
                    // that GrTextureOp skips SkPaint conversion, which normally handles this.
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

                    if (gp.fGeomDomain.isInitialized()) {
                        // Calculate distance from sk_FragCoord to the 4 edges of the domain
                        // and clamp them to (0, 1). Use the minimum of these and the original
                        // coverage. This only has to be done in the exterior triangles, the
                        // interior of the quad geometry can never be clipped by the domain box.
                        args.fFragBuilder->codeAppend("float4 geoDomain;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fGeomDomain, "geoDomain",
                                        Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "if (coverage < 0.5) {"
                                "   float4 dists4 = clamp(float4(1, 1, -1, -1) * "
                                        "(sk_FragCoord.xyxy - geoDomain), 0, 1);"
                                "   float2 dists2 = dists4.xy * dists4.zw;"
                                "   coverage = min(coverage, dists2.x * dists2.y);"
                                "}");
                    }

                    args.fFragBuilder->codeAppendf("%s = half4(half(coverage));",
                                                   args.fOutputCoverage);
                } else {
                    // Set coverage to 1, since it's either non-AA or the coverage was already
                    // folded into the output color
                    SkASSERT(!gp.fGeomDomain.isInitialized());
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                }
            }
            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    friend class ::SkArenaAlloc; // for access to ctor

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(nullptr) {
        SkASSERT(!spec.hasDomain());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(0);
    }

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec,
                                   const GrShaderCaps& caps,
                                   const GrBackendFormat& backendFormat,
                                   const GrSamplerState& samplerState,
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

        // Need a geometry domain when the quads are AA and not rectilinear, since their AA
        // outsetting can go beyond a half pixel.
        if (spec.requiresGeometryDomain()) {
            fGeomDomain = {"geomDomain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
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

        if (spec.hasDomain()) {
            fTexDomain = {"texDomain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        this->setVertexAttributes(&fPosition, 6);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition; // May contain coverage as last channel
    Attribute fCoverage; // Used for non-perspective position to avoid Intel Metal issues
    Attribute fColor; // May have coverage modulated in if the FPs support it
    Attribute fLocalCoord;
    Attribute fGeomDomain; // Screen-space bounding box on geometry+aa outset
    Attribute fTexDomain; // Texture-space bounding box on local coords

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

    typedef GrGeometryProcessor INHERITED;
};

GrGeometryProcessor* MakeProcessor(SkArenaAlloc* arena, const VertexSpec& spec) {
    return QuadPerEdgeAAGeometryProcessor::Make(arena, spec);
}

GrGeometryProcessor* MakeTexturedProcessor(SkArenaAlloc* arena, const VertexSpec& spec,
                                           const GrShaderCaps& caps,
                                           const GrBackendFormat& backendFormat,
                                           const GrSamplerState& samplerState,
                                           const GrSwizzle& swizzle,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform,
                                          Saturate saturate) {
    return QuadPerEdgeAAGeometryProcessor::Make(arena, spec, caps, backendFormat, samplerState,
                                                swizzle, std::move(textureColorSpaceXform),
                                                saturate);
}

} // namespace GrQuadPerEdgeAA
