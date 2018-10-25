/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPerEdgeAAQuadHelper_DEFINED
#define GrPerEdgeAAQuadHelper_DEFINED

#include "GrColor.h"
#include "GrSamplerState.h"
#include "GrTypesPriv.h"
#include "SkPoint.h"
#include "SkPoint3.h"

class GrPerspQuad;

namespace GrPerEdgeAA {

enum class Domain : bool { kNo = false, kYes = true };

// Non-optional field for device position of vertex. N should be 2 or 3
template <int N> struct PositionField {
    static constexpr int kPositionDim = N;
    static constexpr size_t kPositionSize = N * sizeof(float);
    float fPosition[N];
};

// Optional field for vertex color. C should be GrColor or void.
template <typename C> struct ColorField {
    using Color = C;
    static constexpr size_t kColorSize = sizeof(C);
    C fColor;
};
template <> struct ColorField<void> {
    using Color = void;
    static constexpr size_t kColorSize = 0;
};

// Optional field for local coordinates. N should be 0, 2, or 3
template <int N> struct LocalPositionField {
    static constexpr int kLocalPositionDim = N;
    static constexpr size_t kLocalPositionSize = N * sizeof(float);
    float fLocalPosition[N];
};
template <> struct LocalPositionField<0> {
    static constexpr int kLocalPositionDim = 0;
    static constexpr size_t kLocalPositionSize = 0;
};

// Optional field for storing a domain for the local coordinates.
template <Domain D> struct DomainField {
    static constexpr Domain kDomain = D;
    static constexpr size_t kDomainSize = sizeof(SkRect);
    SkRect fDomain;
};
template <> struct DomainField<Domain::kNo> {
    static constexpr Domain kDomain = Domain::kNo;
    static constexpr size_t kDomainSize = 0;
};

// Optional field for storing AA edge equations.
template <GrAA AA> struct AAField {
    static constexpr GrAA kAA = AA;
    static constexpr size_t kAASize = 4 * sizeof(SkPoint3);
    // Edge equation coefficients (not homogeneous coordinates)
    SkPoint3 fEdges[4];
};
template <> struct AAField<GrAA::kNo> {
    static constexpr GrAA kAA = GrAA::kNo;
    static constexpr size_t kAASize = 0;
};

// General vertex definition configured by template arguments
template <int PosDim, typename Color, int LocalPosDim, Domain D, GrAA AA>
struct Vertex : public PositionField<PosDim>, public ColorField<Color>,
                public LocalPositionField<LocalPosDim>, public DomainField<D>, public AAField<AA> {
    // Define offsets to each of the fields based on the inheritance order
    static constexpr size_t kPositionOffset = 0;
    static constexpr size_t kColorOffset = kPositionOffset + PositionField<PosDim>::kPositionSize;
    static constexpr size_t kLocalPositionOffset = kColorOffset + ColorField<Color>::kColorSize;
    static constexpr size_t kDomainOffset = kLocalPositionOffset + LocalPositionField<LocalPosDim>::kLocalPositionSize;
    static constexpr size_t kAAOffset = kDomainOffset + DomainField<D>::kDomainSize;
    // Overall size of the vertex
    static constexpr size_t kVertexSize = kAAOffset + AAField<AA>::kAASize;
};

// The tessellation function is templated based on the specific vertex configuration. However, to
// reduce code size, it is actually implemented by a very general function (ApplyImpl). These
// functions are wrapped in a class so that the general implementation can be made private and the
// public template definition can be public and defined in the header so that no explicit
// instantiation is needed for the many different vertex combinations that are possible.
class Tessellator {
public:
    // Tessellate the given quad specification into the vertices buffer. If the specific vertex
    // type does not use color, local positions, domain, etc. then the passed in values used for
    // that field will be ignored.
    template<typename V>
    static void Apply(V* vertices, const GrPerspQuad& deviceQuad, typename V::Color color,
                      const GrPerspQuad& srcQuad, const SkRect& domain, GrQuadAAFlags aaFlags) {
        static constexpr bool useCoverageAA = V::kAA == GrAA::kYes;
        float localStorage[4 * (V::kPositionDim + V::kLocalPositionDim + (useCoverageAA ? 3 : 0))];
        Tessellator::ApplyImpl(vertices, V::kVertexSize, localStorage,
                deviceQuad, V::kPositionDim, V::kPositionOffset, V::kPositionSize, // PositionField
                &color, V::kColorOffset, V::kColorSize,                            // ColorField
                srcQuad, V::kLocalPositionDim, V::kLocalPositionOffset, V::kLocalPositionSize, // LP
                &domain, V::kDomainOffset, V::kDomainSize,                         // DomainField
                aaFlags, V::kAAOffset, V::kAASize);                                // AAField
    }
private:
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
    static void ApplyImpl(void* vertices, size_t vertexSize, float* localStorage,
            const GrPerspQuad& deviceQuad, int posDim, size_t posOffset, size_t posSize,
            const void* color, size_t colorOffset, size_t colorSize,
            const GrPerspQuad& srcQuad, int srcDim, size_t srcOffset, size_t srcSize,
            const void* domain, size_t domainOffset, size_t domainSize,
            GrQuadAAFlags aaFlags, size_t aaOffset, size_t aaSize);
};

} // namespace GrPerEdgeAA

#endif // GrPerEdgeAAQuadHelper_DEFINED
