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

// Pos is either SkPoint or SkPoint3, depending on if perspective is needed for device space.
template <typename Pos> struct VertexCommon {
    using Position = Pos;
    Position fPosition;
    GrColor fColor;
};

template <typename Pos, typename LocalPos> struct OptionalLocalVertex;
template <typename Pos>
struct OptionalLocalVertex<Pos, void> : VertexCommon<Pos> {
    using LocalPosition = void;
};
template <typename Pos>
struct OptionalLocalVertex<Pos, SkPoint> : VertexCommon<Pos> {
    using LocalPosition = SkPoint;
    SkPoint fLocalPos;
};
template <typename Pos>
struct OptionalLocalVertex<Pos, SkPoint3> : VertexCommon<Pos> {
    using LocalPosition = SkPoint3;
    SkPoint3 fLocalPos;
};

template <typename Pos, typename LocalPos, Domain D> struct OptionalDomainVertex;
template <typename Pos, typename LocalPos>
struct OptionalDomainVertex<Pos, LocalPos, Domain::kNo> : OptionalLocalVertex<Pos, LocalPos> {
    static constexpr Domain kDomain = Domain::kNo;
};
template <typename Pos, typename LocalPos>
struct OptionalDomainVertex<Pos, LocalPos, Domain::kYes> : OptionalLocalVertex<Pos, LocalPos> {
    static constexpr Domain kDomain = Domain::kYes;
    // Used to restrict the src position after interpolation (and perspective division if LocalPos
    // is an SkPoint3).
    SkRect fSrcDomain;
};

template <typename Pos, typename LocalPos, Domain D, GrAA> struct OptionalAAVertex;
template <typename Pos, typename LocalPos, Domain D>
struct OptionalAAVertex<Pos, LocalPos, D, GrAA::kNo>
        : OptionalDomainVertex<Pos, LocalPos, D> {
    static constexpr GrAA kAA = GrAA::kNo;
};
template <typename Pos, typename LocalPos, Domain D>
struct OptionalAAVertex<Pos, LocalPos, D, GrAA::kYes>
        : OptionalDomainVertex<Pos, LocalPos, D> {
    static constexpr GrAA kAA = GrAA::kYes;
    // Edge equation coefficients (not homogeneous coordinates)
    SkPoint3 fEdges[4];
};

template <typename Pos, typename LocalPos, Domain D, GrAA AA>
using Vertex = OptionalAAVertex<Pos, LocalPos, D, AA>;

// The optimized, textured quad (for GrTextureOp) assumes the local coordinates remain a rect,
// hence srcRect is an SkRect and the LocalPos template type is fixed to SkPoint. It does however
// support having an explicit domain or not.
template<typename Pos, Domain D, GrAA AA>
void TesselateTexturedQuad(Vertex<Pos, SkPoint, D, AA>* vertices, GrQuadAAFlags aaFlags,
                           Domain constrainDomain, const GrPerspQuad& deviceQuad,
                           const SkRect& srcRect, GrColor color, GrSurfaceOrigin origin,
                           GrSamplerState::Filter filter, float iw, float ih);

// The untextured quad allows for no, 2D, and perspective local coordinates, but it does not store
// a domain at all.
template<typename Pos, typename LocalPos, GrAA AA>
void TesselateQuad(Vertex<Pos, LocalPos, Domain::kNo, AA>* vertices, GrQuadAAFlags aaFlags,
                   const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad, GrColor color);

} // namespace GrPerEdgeAA

#endif // GrPerEdgeAAQuadHelper_DEFINED
