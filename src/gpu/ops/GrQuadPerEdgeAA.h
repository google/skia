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
#include "GrSamplerState.h"
#include "GrTypesPriv.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "SkPoint.h"
#include "SkPoint3.h"

class GrGLSLColorSpaceXformHelper;
class GrPerspQuad;

class GrQuadPerEdgeAA {
public:
    enum class Domain : bool { kNo = false, kYes = true };

    // The vertex template provides a clean way of specifying the layout and components of a vertex
    // for a per-edge aa quad. However, because there are so many permutations possible, the struct
    // is defined this way to take away all layout control from the compiler and make
    // sure that it matches what we need to send to the GPU.
    //
    // It is expected that most code using these vertices will only need to call the templated
    // Tessellate() function with an appropriately sized vertex buffer and not need to modify or
    // read the fields of a particular vertex.
    template <int PosDim, typename C, int LocalPosDim, Domain D, GrAA AA>
    struct Vertex {
        using Color = C;
        static constexpr GrAA kAA = AA;
        static constexpr Domain kDomain = D;
        static constexpr size_t kPositionDim = PosDim;
        static constexpr size_t kLocalPositionDim = LocalPosDim;

        static constexpr size_t kPositionOffset = 0;
        static constexpr size_t kPositionSize = PosDim * sizeof(float);

        static constexpr size_t kColorOffset = kPositionOffset + kPositionSize;
        static constexpr size_t kColorSize = sizeof(Color);

        static constexpr size_t kLocalPositionOffset = kColorOffset + kColorSize;
        static constexpr size_t kLocalPositionSize = LocalPosDim * sizeof(float);

        static constexpr size_t kDomainOffset = kLocalPositionOffset + kLocalPositionSize;
        static constexpr size_t kDomainSize = D == Domain::kYes ? sizeof(SkRect) : 0;

        static constexpr size_t kAAOffset = kDomainOffset + kDomainSize;
        static constexpr size_t kAASize = AA == GrAA::kYes ? 4 * sizeof(SkPoint3) : 0;

        static constexpr size_t kVertexSize = kAAOffset + kAASize;

        // Make sure sizeof(Vertex<...>) == kVertexSize
        char fData[kVertexSize];
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

        GPAttributes(int posDim, int localDim, bool hasColor, GrAAType aa, Domain domain);

        const Attribute& positions() const { return fPositions; }
        const Attribute& colors() const { return fColors; }
        const Attribute& localCoords() const { return fLocalCoords; }
        const Attribute& domain() const { return fDomain; }
        const Attribute& edges(int i) const { return fAAEdges[i]; }

        bool hasVertexColors() const { return fColors.isInitialized(); }

        bool usesCoverageAA() const { return fAAEdges[0].isInitialized(); }

        bool hasLocalCoords() const { return fLocalCoords.isInitialized(); }

        bool hasDomain() const { return fDomain.isInitialized(); }

        bool needsPerspectiveInterpolation() const;

        const Attribute* attributes() const { return &fPositions; }
        int attributeCount() const { return 8; }

        uint32_t getKey() const;

        // Functions to be called at appropriate times in a processor's onEmitCode() block. These
        // are separated into discrete pieces so that they can be interleaved with the rest of the
        // processor's shader code as needed. The functions take char* arguments for the names of
        // variables the emitted code must declare, so that the calling GP can ensure there's no
        // naming conflicts with their own code.

        void emitColor(GrGLSLPrimitiveProcessor::EmitArgs& args,
                       GrGLSLColorSpaceXformHelper* colorSpaceXformHelper,
                       const char* colorVarName) const;

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
        Attribute fPositions;   // named "position" in SkSL
        Attribute fColors;      // named "color" in SkSL
        Attribute fLocalCoords; // named "localCoord" in SkSL
        Attribute fDomain;      // named "domain" in SkSL
        Attribute fAAEdges[4];  // named "aaEdgeX" for X = 0,1,2,3
    };

    // Tessellate the given quad specification into the vertices buffer. If the specific vertex
    // type does not use color, local positions, domain, etc. then the passed in values used for
    // that field will be ignored.
    template<typename V>
    static void Tessellate(V* vertices, const GrPerspQuad& deviceQuad, typename V::Color color,
                           const GrPerspQuad& srcQuad, const SkRect& domain, GrQuadAAFlags aa) {
        static_assert(sizeof(V) == V::kVertexSize, "Incorrect vertex size");
        static constexpr bool useCoverageAA = V::kAA == GrAA::kYes;
        float localStorage[4 * (V::kPositionDim + V::kLocalPositionDim + (useCoverageAA ? 3 : 0))];
        TessellateImpl(vertices, V::kVertexSize, localStorage,
                deviceQuad, V::kPositionDim, V::kPositionOffset, V::kPositionSize,
                &color, V::kColorOffset, V::kColorSize,
                srcQuad, V::kLocalPositionDim, V::kLocalPositionOffset, V::kLocalPositionSize,
                &domain, V::kDomainOffset, V::kDomainSize,
                aa, V::kAAOffset, V::kAASize);
    }

private:
    // Don't let the "namespace" class be instantiated
    GrQuadPerEdgeAA();

    // Internal implementation that can handle all vertex template variations without being
    // replicated by the template in order to keep code size down.
    //
    // This uses the field sizes to determine if particular data needs to be computed. The arguments
    // are arranged so that the data and field specification match the field declaration order of
    // the vertex type (pos, color, localPos, domain, aa).
    //
    // localStorage must be have a length > 4 * (devDimCt + srcDimCt + (aa ? 3 : 0)) and is assumed
    // to be a pointer to a local variable in the wrapping template's stack. This is done instead of
    // always allocating 36 floats in this function (36 is maximum needed). The minimum needed for a
    // non-AA 2D quad with no local coordinates is just 8.
    static void TessellateImpl(void* vertices, size_t vertexSize, float* localStorage,
            const GrPerspQuad& deviceQuad, int posDim, size_t posOffset, size_t posSize,
            const void* color, size_t colorOffset, size_t colorSize,
            const GrPerspQuad& srcQuad, int srcDim, size_t srcOffset, size_t srcSize,
            const void* domain, size_t domainOffset, size_t domainSize,
            GrQuadAAFlags aaFlags, size_t aaOffset, size_t aaSize);
};

#endif // GrQuadPerEdgeAA_DEFINED
