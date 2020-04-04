/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadPerEdgeAA_DEFINED
#define GrQuadPerEdgeAA_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/geometry/GrQuad.h"
#include "src/gpu/geometry/GrQuadUtils.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrTextureOp.h"

class GrCaps;
class GrColorSpaceXform;
class GrShaderCaps;
struct GrVertexWriter;

namespace GrQuadPerEdgeAA {
    using Saturate = GrTextureOp::Saturate;

    enum class CoverageMode { kNone, kWithPosition, kWithColor };
    enum class Domain : bool { kNo = false, kYes = true };
    enum class ColorType { kNone, kByte, kFloat, kLast = kFloat };
    static const int kColorTypeCount = static_cast<int>(ColorType::kLast) + 1;

    enum class IndexBufferOption {
        kPictureFramed,    // geometrically AA'd   -> 8 verts/quad + an index buffer
        kIndexedRects,     // non-AA'd but indexed -> 4 verts/quad + an index buffer
        kTriStrips,        // non-AA'd             -> 4 verts/quad but no index buffer
        kLast = kTriStrips
    };
    static const int kIndexBufferOptionCount = static_cast<int>(IndexBufferOption::kLast) + 1;

    IndexBufferOption CalcIndexBufferOption(GrAAType aa, int numQuads);

    // Gets the minimum ColorType that can represent a color.
    ColorType MinColorType(SkPMColor4f);

    // Specifies the vertex configuration for an op that renders per-edge AA quads. The vertex
    // order (when enabled) is device position, color, local position, domain, aa edge equations.
    // This order matches the constructor argument order of VertexSpec and is the order that
    // GPAttributes maintains. If hasLocalCoords is false, then the local quad type can be ignored.
    struct VertexSpec {
    public:
        VertexSpec()
                : fDeviceQuadType(0)     // kAxisAligned
                , fLocalQuadType(0)      // kAxisAligned
                , fIndexBufferOption(0)  // kPictureFramed
                , fHasLocalCoords(false)
                , fColorType(0)          // kNone
                , fHasDomain(false)
                , fUsesCoverageAA(false)
                , fCompatibleWithCoverageAsAlpha(false)
                , fRequiresGeometryDomain(false) {}

        VertexSpec(GrQuad::Type deviceQuadType, ColorType colorType, GrQuad::Type localQuadType,
                   bool hasLocalCoords, Domain domain, GrAAType aa, bool coverageAsAlpha,
                   IndexBufferOption indexBufferOption)
                : fDeviceQuadType(static_cast<unsigned>(deviceQuadType))
                , fLocalQuadType(static_cast<unsigned>(localQuadType))
                , fIndexBufferOption(static_cast<unsigned>(indexBufferOption))
                , fHasLocalCoords(hasLocalCoords)
                , fColorType(static_cast<unsigned>(colorType))
                , fHasDomain(static_cast<unsigned>(domain))
                , fUsesCoverageAA(aa == GrAAType::kCoverage)
                , fCompatibleWithCoverageAsAlpha(coverageAsAlpha)
                , fRequiresGeometryDomain(aa == GrAAType::kCoverage &&
                                          deviceQuadType > GrQuad::Type::kRectilinear) { }

        GrQuad::Type deviceQuadType() const { return static_cast<GrQuad::Type>(fDeviceQuadType); }
        GrQuad::Type localQuadType() const { return static_cast<GrQuad::Type>(fLocalQuadType); }
        IndexBufferOption indexBufferOption() const {
            return static_cast<IndexBufferOption>(fIndexBufferOption);
        }
        bool hasLocalCoords() const { return fHasLocalCoords; }
        ColorType colorType() const { return static_cast<ColorType>(fColorType); }
        bool hasVertexColors() const { return ColorType::kNone != this->colorType(); }
        bool hasDomain() const { return fHasDomain; }
        bool usesCoverageAA() const { return fUsesCoverageAA; }
        bool compatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }
        bool requiresGeometryDomain() const { return fRequiresGeometryDomain; }
        // Will always be 2 or 3
        int deviceDimensionality() const;
        // Will always be 0 if hasLocalCoords is false, otherwise will be 2 or 3
        int localDimensionality() const;

        int verticesPerQuad() const { return fUsesCoverageAA ? 8 : 4; }

        CoverageMode coverageMode() const;
        size_t vertexSize() const;

        bool needsIndexBuffer() const { return this->indexBufferOption() !=
                                               IndexBufferOption::kTriStrips; }

        GrPrimitiveType primitiveType() const {
            switch (this->indexBufferOption()) {
                case IndexBufferOption::kPictureFramed: return GrPrimitiveType::kTriangles;
                case IndexBufferOption::kIndexedRects:  return GrPrimitiveType::kTriangles;
                case IndexBufferOption::kTriStrips:     return GrPrimitiveType::kTriangleStrip;
            }

            SkUNREACHABLE;
        }

    private:
        static_assert(GrQuad::kTypeCount <= 4, "GrQuad::Type doesn't fit in 2 bits");
        static_assert(kColorTypeCount <= 4, "Color doesn't fit in 2 bits");
        static_assert(kIndexBufferOptionCount <= 4, "IndexBufferOption doesn't fit in 2 bits");

        unsigned fDeviceQuadType: 2;
        unsigned fLocalQuadType: 2;
        unsigned fIndexBufferOption: 2;
        unsigned fHasLocalCoords: 1;
        unsigned fColorType : 2;
        unsigned fHasDomain: 1;
        unsigned fUsesCoverageAA: 1;
        unsigned fCompatibleWithCoverageAsAlpha: 1;
        // The geometry domain serves to clip off pixels touched by quads with sharp corners that
        // would otherwise exceed the miter limit for the AA-outset geometry.
        unsigned fRequiresGeometryDomain: 1;
    };

    // A Tessellator is responsible for processing a series of device+local GrQuads into a VBO,
    // as specified by a VertexSpec. This vertex data can then be processed by a GP created with
    // MakeProcessor and/or MakeTexturedProcessor.
    class Tessellator {
    public:
        explicit Tessellator(const VertexSpec& spec, char* vertices);

        // Calculates (as needed) inset and outset geometry for anti-aliasing, and appends all
        // necessary position and vertex attributes required by this Tessellator's VertexSpec into
        // the 'vertices' the Tessellator was called with. The insetting and outsetting may
        // damage the provided GrQuads (as this is intended to work with GrQuadBuffer::Iter).
        // 'localQuad' can be null if the VertexSpec does not use local coords.
        void append(GrQuad* deviceQuad, GrQuad* localQuad,
                    const SkPMColor4f& color, const SkRect& uvDomain, GrQuadAAFlags aaFlags);

        SkDEBUGCODE(char* vertices() const { return (char*) fVertexWriter.fPtr; })

    private:
        // VertexSpec defines many unique ways to write vertex attributes, which can be handled
        // generically by branching per-quad based on the VertexSpec. However, there are several
        // specs that appear in the wild far more frequently, so they use explicit WriteQuadProcs
        // that have no branches.
        typedef void (*WriteQuadProc)(GrVertexWriter* vertices, const VertexSpec& spec,
                                      const GrQuad* deviceQuad, const GrQuad* localQuad,
                                      const float coverage[4], const SkPMColor4f& color,
                                      const SkRect& geomDomain, const SkRect& texDomain);
        static WriteQuadProc GetWriteQuadProc(const VertexSpec& spec);

        GrQuadUtils::TessellationHelper fAAHelper;
        VertexSpec                      fVertexSpec;
        GrVertexWriter                  fVertexWriter;
        WriteQuadProc                   fWriteProc;
    };

    GrGeometryProcessor* MakeProcessor(SkArenaAlloc*, const VertexSpec&);

    GrGeometryProcessor* MakeTexturedProcessor(
            SkArenaAlloc*, const VertexSpec&, const GrShaderCaps&, const GrBackendFormat&,
            const GrSamplerState&, const GrSwizzle&,
            sk_sp<GrColorSpaceXform> textureColorSpaceXform, Saturate);

    // This method will return the correct index buffer for the specified indexBufferOption.
    // It will, correctly, return nullptr if the indexBufferOption is kTriStrips.
    sk_sp<const GrBuffer> GetIndexBuffer(GrMeshDrawOp::Target*, IndexBufferOption);

    // What is the maximum number of quads allowed for the specified indexBuffer option?
    int QuadLimit(IndexBufferOption);

    // This method will configure the vertex and index data of the provided 'mesh' to comply
    // with the indexing method specified in the vertexSpec. It is up to the calling code
    // to allocate and fill in the vertex data and acquire the correct indexBuffer if it is needed.
    //
    // @param runningQuadCount  the number of quads already stored in 'vertexBuffer' and
    //                          'indexBuffer' e.g., different GrMeshes have already been placed in
    //                          the buffers to allow dynamic state changes.
    // @param quadCount         the number of quads that will be drawn by the provided 'mesh'.
    //                          A subsequent ConfigureMesh call would the use
    //                          'runningQuadCount' + 'quadCount' for its new 'runningQuadCount'.
    void ConfigureMesh(const GrCaps&, GrMesh*, const VertexSpec&, int runningQuadCount,
                       int quadCount, int maxVerts, sk_sp<const GrBuffer> vertexBuffer,
                       sk_sp<const GrBuffer> indexBuffer, int absVertBufferOffset);

} // namespace GrQuadPerEdgeAA

#endif // GrQuadPerEdgeAA_DEFINED
