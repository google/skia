/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrQuadPerEdgeAA_DEFINED
#define GrQuadPerEdgeAA_DEFINED

#include "GrColor.h"
#include "GrGeometryProcessor.h"
#include "GrQuad.h"
#include "GrSamplerState.h"
#include "GrTypesPriv.h"
#include "SkPoint.h"
#include "SkPoint3.h"

class GrColorSpaceXform;
class GrShaderCaps;

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

    sk_sp<GrGeometryProcessor> MakeProcessor(const VertexSpec& spec);

    sk_sp<GrGeometryProcessor> MakeTexturedProcessor(const VertexSpec& spec,
            const GrShaderCaps& caps, GrTextureType textureType, GrPixelConfig textureConfig,
            const GrSamplerState& samplerState, uint32_t extraSamplerKey,
            sk_sp<GrColorSpaceXform> textureColorSpaceXform);

    // Fill vertices with the vertex data needed to represent the given quad. The device position,
    // local coords, vertex color, domain, and edge coefficients will be written and/or computed
    // based on the configuration in the vertex spec; if that attribute is disabled in the spec,
    // then its corresponding function argument is ignored.
    //
    // Returns the advanced pointer in vertices.
    void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                     const SkPMColor4f& color, const GrPerspQuad& localQuad, const SkRect& domain,
                     GrQuadAAFlags aa);

} // namespace GrQuadPerEdgeAA

#endif // GrQuadPerEdgeAA_DEFINED
