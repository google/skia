/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadPerEdgeAA_DEFINED
#define GrQuadPerEdgeAA_DEFINED

#include "GrColor.h"
#include "GrPrimitiveProcessor.h"
#include "GrQuad.h"
#include "GrSamplerState.h"
#include "GrTypesPriv.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "SkPoint.h"
#include "SkPoint3.h"

class GrGLSLColorSpaceXformHelper;

namespace GrQuadPerEdgeAA {

    enum class Domain : bool { kNo = false, kYes = true };
    enum class ColorType { kNone, kByte, kHalf, kLast = kHalf };
    static const int kColorTypeCount = static_cast<int>(ColorType::kLast) + 1;

    // Specifies the vertex configuration for an op that renders per-edge AA quads. The vertex
    // order (when enabled) is device position, color, local position, domain, aa edge equations.
    // This order matches the constructor argument order of VertexSpec and is the order that
    // GPAttributes maintains. If hasLocalCoords is false, then the local quad type can be ignored.
    struct VertexSpec {
    public:
        VertexSpec(GrQuadType deviceQuadType, ColorType colorType, GrQuadType localQuadType,
                   bool hasLocalCoords, Domain domain, GrAAType aa)
                : fDeviceQuadType(static_cast<unsigned>(deviceQuadType))
                , fLocalQuadType(static_cast<unsigned>(localQuadType))
                , fHasLocalCoords(hasLocalCoords)
                , fColorType(static_cast<unsigned>(colorType))
                , fHasDomain(static_cast<unsigned>(domain))
                , fUsesCoverageAA(aa == GrAAType::kCoverage) { }

        GrQuadType deviceQuadType() const { return static_cast<GrQuadType>(fDeviceQuadType); }
        GrQuadType localQuadType() const { return static_cast<GrQuadType>(fLocalQuadType); }
        bool hasLocalCoords() const { return fHasLocalCoords; }
        ColorType colorType() const { return static_cast<ColorType>(fColorType); }
        bool hasVertexColors() const { return ColorType::kNone != this->colorType(); }
        bool hasDomain() const { return fHasDomain; }
        bool usesCoverageAA() const { return fUsesCoverageAA; }

        // Will always be 2 or 3
        int deviceDimensionality() const;
        // Will always be 0 if hasLocalCoords is false, otherwise will be 2 or 3
        int localDimensionality() const;

    private:
        static_assert(kGrQuadTypeCount <= 4, "GrQuadType doesn't fit in 2 bits");
        static_assert(kColorTypeCount <= 4, "Color doesn't fit in 2 bits");

        unsigned fDeviceQuadType: 2;
        unsigned fLocalQuadType: 2;
        unsigned fHasLocalCoords: 1;
        unsigned fColorType : 2;
        unsigned fHasDomain: 1;
        unsigned fUsesCoverageAA: 1;
    };

    // Utility class that manages the attribute state necessary to render a particular batch of
    // quads. It is similar to a geometry processor but is meant to be included in a has-a
    // relationship by specialized GP's that provide further functionality on top of the per-edge AA
    // coverage.
    //
    // For performance reasons, this uses fixed names for the attribute variables; since it defines
    // the majority of attributes a GP will likely need, this shouldn't be too limiting.
    //
    // In terms of responsibilities, the actual geometry processor must still call emitTransforms(),
    // using the localCoords() attribute as the 4th argument; it must set the transform data helper
    // to use the identity matrix; it must manage the color space transform for the quad's paint
    // color; it should include getKey() in the geometry processor's key builder; and it should
    // add these attributes at construction time.
    class GPAttributes {
    public:
        using Attribute = GrPrimitiveProcessor::Attribute;

        GPAttributes(const VertexSpec& vertexSpec);

        const Attribute& positions() const { return fPositions; }
        const Attribute& colors() const { return fColors; }
        const Attribute& localCoords() const { return fLocalCoords; }
        const Attribute& domain() const { return fDomain; }
        const Attribute& edgeDistances() const { return fAAEdgeDistances; }

        bool hasVertexColors() const { return fColors.isInitialized(); }

        bool usesCoverageAA() const { return fAAEdgeDistances.isInitialized(); }

        bool hasLocalCoords() const { return fLocalCoords.isInitialized(); }

        bool hasDomain() const { return fDomain.isInitialized(); }

        bool needsPerspectiveInterpolation() const;

        const Attribute* attributes() const { return &fPositions; }
        int attributeCount() const {  return 5; }

        uint32_t getKey() const;

        // Functions to be called at appropriate times in a processor's onEmitCode() block. These
        // are separated into discrete pieces so that they can be interleaved with the rest of the
        // processor's shader code as needed. The functions take char* arguments for the names of
        // variables the emitted code must declare, so that the calling GP can ensure there's no
        // naming conflicts with their own code.

        void emitColor(GrGLSLPrimitiveProcessor::EmitArgs& args, const char* colorVarName) const;

        // localCoordName will be declared as a float2, with any domain applied after any
        // perspective division is performed.
        //
        // Note: this should only be used if the local coordinates need to be passed separately
        // from the standard coord transform process that is used by FPs.
        // FIXME: This can go in two directions from here, if GrTextureOp stops needing per-quad
        // domains it can be removed and GrTextureOp rewritten to use coord transforms. Or
        // emitTransform() in the primitive builder can be updated to have a notion of domain for
        // local coords, and all domain-needing code (blurs, filters, etc.) can switch to that magic
        void emitExplicitLocalCoords(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                     const char* localCoordName, const char* domainVarName) const;

        void emitCoverage(GrGLSLPrimitiveProcessor::EmitArgs& args, const char* edgeDistName) const;
    private:
        Attribute fPositions;       // named "position" in SkSL
        Attribute fColors;          // named "color" in SkSL
        Attribute fLocalCoords;     // named "localCoord" in SkSL
        Attribute fDomain;          // named "domain" in SkSL
        Attribute fAAEdgeDistances; // named "aaEdgeDist" in SkSL
    };

    // Fill vertices with the vertex data needed to represent the given quad. The device position,
    // local coords, vertex color, domain, and edge coefficients will be written and/or computed
    // based on the configuration in the vertex spec; if that attribute is disabled in the spec,
    // then its corresponding function argument is ignored.
    //
    // Returns the advanced pointer in vertices.
    // TODO4F: Switch GrColor to GrVertexColor
    void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                     const SkPMColor4f& color, const GrPerspQuad& localQuad, const SkRect& domain,
                     GrQuadAAFlags aa);

} // namespace GrQuadPerEdgeAA

#endif // GrQuadPerEdgeAA_DEFINED
