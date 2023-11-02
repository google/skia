/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/PerEdgeAAQuadRenderStep.h"

#include "src/base/SkVx.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

// This RenderStep is specialized to draw filled rectangles with per-edge AA.
//
// Each of these "primitives" is represented by a single instance. The instance attributes are
// flexible enough to describe per-edge AA quads without relying on uniforms to define its
// operation. The attributes encode shape as follows:
//
// float4 edgeFlags - per-edge AA defined by each component: aa != 0.
// float4 quadXs - these values provide the X coordinates of the quadrilateral in top-left CW order.
// float4 quadYs - these values provide the Y coordinates of the quadrilateral.
//
// From the other direction, per-edge AA quads produce instance values like:
//  - [aa(t,r,b,l) ? 255 : 0]   [xs(tl,tr,br,bl)]     [ys(tl,tr,br,bl)]
//
// From this encoding, data can be unpacked for each corner, which are equivalent under
// rotational symmetry. Per-edge quads are always mitered and fill the interior, but the
// vertices are placed such that the edge coverage ramps can collapse to 0 area on non-AA edges.
//
// The vertices that describe each corner are placed so that edges and miters calculate
// coverage by interpolating a varying and then clamping in the fragment shader. Triangles that
// cover the inner and outer curves calculate distance to the curve within the fragment shader.
//
// See https://docs.google.com/presentation/d/1MCPstNsSlDBhR8CrsJo0r-cZNbu-sEJEvU9W94GOJoY/edit?usp=sharing
// for diagrams and explanation of how the geometry is defined.
//
// PerEdgeAAQuadRenderStep uses the common technique of approximating distance to the level set by
// one expansion of the Taylor's series for the level set's equation. Given a level set function
// C(x,y), this amounts to calculating C(px,py)/|∇C(px,py)|. For the straight edges the level set
// is linear and calculated in the vertex shader and then interpolated exactly over the rectangle.
// This provides distances to all four exterior edges within the fragment shader and allows it to
// reconstruct a relative position per elliptical corner. Unfortunately this requires the fragment
// shader to calculate the length of the gradient for straight edges instead of interpolating
// exact device-space distance.
//
// Unlike AnalyticRRectRenderStep, for per-edge AA quads it's valid to have each pixel calculate a
// single corner's coverage that's controlled via the vertex shader. Any bias is a constant 1/2,
// so this is also added in the vertex shader.
//
// Analytic derivatives are used so that a single pipeline can be used regardless of HW derivative
// support or for geometry that would prove difficult for forward differencing. The device-space
// gradient for ellipses is calculated per-pixel by transforming a per-pixel local gradient vector
// with the Jacobian of the inverse local-to-device transform:
//
// (px,py) is the projected point of (u,v) transformed by a 3x3 matrix, M:
//                [x(u,v) / w(u,v)]       [x]   [m00 m01 m02] [u]
//      (px,py) = [y(u,v) / w(u,v)] where [y] = [m10 m11 m12]X[v] = M*(u,v,1)
//                                        [w]   [m20 m21 m22] [1]
//
// C(px,py) can be defined in terms of a local Cl(u,v) as C(px,py) = Cl(p^-1(px,py)), where p^-1 =
//
//               [x'(px,py) / w'(px,py)]       [x']   [m00' m01' * m02'] [px]
//      (u,v) =  [y'(px,py) / w'(px,py)] where [y'] = [m10' m11' * m12']X[py] = M^-1*(px,py,0,1)
//                                             [w']   [m20' m21' * m22'] [ 1]
//
// Note that if the 3x3 M was arrived by dropping the 3rd row and column from a 4x4 since we assume
// a local 3rd coordinate of 0, M^-1 is not equal to the 4x4 inverse with dropped rows and columns.
//
// Using the chain rule, then ∇C(px,py)
//   =  ∇Cl(u,v)X[1/w'(px,py)     0       -x'(px,py)/w'(px,py)^2] [m00' m01']
//               [    0       1/w'(px,py) -y'(px,py)/w'(px,py)^2]X[m10' m11']
//                                                                [m20' m21']
//
//   = 1/w'(px,py)*∇Cl(u,v)X[1 0 -x'(px,py)/w'(px,py)] [m00' m01']
//                          [0 1 -y'(px,py)/w'(px,py)]X[m10' m11']
//                                                     [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[1 0 0 -u] [m00' m01']
//                     [0 1 0 -v]X[m10' m11']
//                                [m20' m21']
//
//   = w(u,v)*∇Cl(u,v)X[m00'-m20'u m01'-m21'u]
//                     [m10'-m20'v m11'-m21'v]
//
// The vertex shader calculates the rightmost 2x2 matrix and interpolates it across the shape since
// each component is linear in (u,v). ∇Cl(u,v) is evaluated per pixel in the fragment shader and
// depends on which corner and edge being evaluated. w(u,v) is the device-space W coordinate, so
// its reciprocal is provided in sk_FragCoord.w.
namespace skgpu::graphite {

using AAFlags = EdgeAAQuad::Flags;

static float local_aa_radius(const Transform& t, const SkV2& p) {
    // TODO: This should be the logic for Transform::scaleFactor()
    //              [m00 m01 * m03]                                 [f(u,v)]
    // Assuming M = [m10 m11 * m13], define the projected p'(u,v) = [g(u,v)] where
    //              [ *   *  *  * ]
    //              [m30 m31 * m33]
    //                                                        [x]     [u]
    // f(u,v) = x(u,v) / w(u,v), g(u,v) = y(u,v) / w(u,v) and [y] = M*[v]
    //                                                        [*] =   [0]
    //                                                        [w]     [1]
    //
    // x(u,v) = m00*u + m01*v + m03
    // y(u,v) = m10*u + m11*v + m13
    // w(u,v) = m30*u + m31*v + m33
    //
    // dx/du = m00, dx/dv = m01,
    // dy/du = m10, dy/dv = m11
    // dw/du = m30, dw/dv = m31
    //
    // df/du = (dx/du*w - x*dw/du)/w^2 = (m00*w - m30*x)/w^2 = (m00 - m30*f)/w
    // df/dv = (dx/dv*w - x*dw/dv)/w^2 = (m01*w - m31*x)/w^2 = (m01 - m31*f)/w
    // dg/du = (dy/du*w - y*dw/du)/w^2 = (m10*w - m30*y)/w^2 = (m10 - m30*g)/w
    // dg/dv = (dy/dv*w - y*dw/du)/w^2 = (m11*w - m31*y)/w^2 = (m11 - m31*g)/w
    //
    // Singular values of [df/du df/dv] define perspective correct minimum and maximum scale factors
    //                    [dg/du dg/dv]
    // for M evaluated at  (u,v)
    const SkM44& matrix = t.matrix();
    SkV4 devP = matrix.map(p.x, p.y, 0.f, 1.f);

    const float dxdu = matrix.rc(0,0);
    const float dxdv = matrix.rc(0,1);
    const float dydu = matrix.rc(1,0);
    const float dydv = matrix.rc(1,1);
    const float dwdu = matrix.rc(3,0);
    const float dwdv = matrix.rc(3,1);

    float invW2 = sk_ieee_float_divide(1.f, (devP.w * devP.w));
    // non-persp has invW2 = 1, devP.w = 1, dwdu = 0, dwdv = 0
    float dfdu = (devP.w*dxdu - devP.x*dwdu) * invW2; // non-persp -> dxdu -> m00
    float dfdv = (devP.w*dxdv - devP.x*dwdv) * invW2; // non-persp -> dxdv -> m01
    float dgdu = (devP.w*dydu - devP.y*dwdu) * invW2; // non-persp -> dydu -> m10
    float dgdv = (devP.w*dydv - devP.y*dwdv) * invW2; // non-persp -> dydv -> m11

    // no-persp, these are the singular values of [m00,m01][m10,m11], which is just the upper 2x2
    // and equivalent to SkMatrix::getMinmaxScales().
    float s1 = dfdu*dfdu + dfdv*dfdv + dgdu*dgdu + dgdv*dgdv;

    float e = dfdu*dfdu + dfdv*dfdv - dgdu*dgdu - dgdv*dgdv;
    float f = dfdu*dgdu + dfdv*dgdv;
    float s2 = SkScalarSqrt(e*e + 4*f*f);

    float singular1 = SkScalarSqrt(0.5f * (s1 + s2));
    float singular2 = SkScalarSqrt(0.5f * (s1 - s2));

    // singular1 and 2 represent the minimum and maximum scale factors at that transformed point.
    // Moving 1 from 'p' before transforming will move at least minimum and at most maximum from
    // the transformed point. Thus moving between [1/max, 1/min] pre-transformation means post
    // transformation moves between [1,max/min] so using 1/min as the local AA radius ensures that
    // the post-transformed point is at least 1px away from the original.
    float aaRadius = sk_ieee_float_divide(1.f, std::min(singular1, singular2));
    if (sk_float_isfinite(aaRadius)) {
        return aaRadius;
    } else {
        // Treat NaNs and infinities as +inf, which will always trigger the inset self-intersection
        // logic that snaps inner vertices to the center instead of insetting by the local AA radius
        return SK_FloatInfinity;
    }
}

static float local_aa_radius(const Transform& t, const Rect& bounds) {
    // Use the maximum radius of the 4 corners so that every local vertex uses the same offset
    // even if it's more conservative on some corners (when the min/max scale isn't constant due
    // to perspective).
    if (t.type() < Transform::Type::kProjection) {
        // Scale factors are constant, so the point doesn't really matter
        return local_aa_radius(t, SkV2{0.f, 0.f});
    } else {
        // TODO can we share calculation here?
        float tl = local_aa_radius(t, SkV2{bounds.left(), bounds.top()});
        float tr = local_aa_radius(t, SkV2{bounds.right(), bounds.top()});
        float br = local_aa_radius(t, SkV2{bounds.right(), bounds.bot()});
        float bl = local_aa_radius(t, SkV2{bounds.left(), bounds.bot()});
        return std::max(std::max(tl, tr), std::max(bl, br));
    }
}

static bool is_clockwise(const EdgeAAQuad& quad) {
    if (quad.isRect()) {
        return true; // by construction, these are always locally clockwise
    }

    // This assumes that each corner has a consistent winding, which is the case for convex inputs,
    // which is an assumption of the per-edge AA API. Check the sign of cross product between the
    // first two edges.
    const skvx::float4& xs = quad.xs();
    const skvx::float4& ys = quad.ys();

    float winding = (xs[0] - xs[3])*(ys[1] - ys[0]) - (ys[0] - ys[3])*(xs[1] - xs[0]);
    if (winding == 0.f) {
        // The input possibly forms a triangle with duplicate vertices, so check the opposite corner
        winding = (xs[2] - xs[1])*(ys[3] - ys[2]) - (ys[2] - ys[1])*(xs[3] - xs[2]);
    }

    // At this point if winding is < 0, the quad's vertices are CCW. If it's still 0, the vertices
    // form a line, in which case the vertex shader constructs a correct CW winding. Otherwise,
    // the quad or triangle vertices produce a positive winding and are CW.
    return winding >= 0.f;
}

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    SkV2 fNormal;
};

// Allowed values for the center weight instance value (selected at record time based on style
// and transform), and are defined such that when (insance-weight > vertex-weight) is true, the
// vertex should be snapped to the center instead of its regular calculation.
static constexpr int kCornerVertexCount = 4; // sk_VertexID is divided by this in SkSL
static constexpr int kVertexCount = 4 * kCornerVertexCount;
static constexpr int kIndexCount = 29;

static void write_index_buffer(VertexWriter writer) {
    static constexpr uint16_t kTL = 0 * kCornerVertexCount;
    static constexpr uint16_t kTR = 1 * kCornerVertexCount;
    static constexpr uint16_t kBR = 2 * kCornerVertexCount;
    static constexpr uint16_t kBL = 3 * kCornerVertexCount;

    static const uint16_t kIndices[kIndexCount] = {
        // Exterior AA ramp outset
        kTL+1,kTL+2,kTL+3,kTR+0,kTR+3,kTR+1,
        kTR+1,kTR+2,kTR+3,kBR+0,kBR+3,kBR+1,
        kBR+1,kBR+2,kBR+3,kBL+0,kBL+3,kBL+1,
        kBL+1,kBL+2,kBL+3,kTL+0,kTL+3,kTL+1,
        kTL+3,
        // Fill triangles
        kTL+3,kTR+3,kBL+3,kBR+3
    };

    writer << kIndices;
}

static void write_vertex_buffer(VertexWriter writer) {
    static constexpr float kHR2 = 0.5f * SK_FloatSqrt2; // "half root 2"

    // This template is repeated 4 times in the vertex buffer, for each of the four corners.
    // The vertex ID is used to lookup per-corner instance properties such as positions,
    // but otherwise this vertex data produces a consistent clockwise mesh from
    // TL -> TR -> BR -> BL.
    static constexpr Vertex kCornerTemplate[kCornerVertexCount] = {
        // Normals for device-space AA outsets from outer curve
        { {1.0f, 0.0f} },
        { {kHR2, kHR2} },
        { {0.0f, 1.0f} },

        // Normal for outer anchor (zero length to signal no local or device-space normal outset)
        { {0.0f, 0.0f} },
    };

    writer << kCornerTemplate  // TL
           << kCornerTemplate  // TR
           << kCornerTemplate  // BR
           << kCornerTemplate; // BL
}

PerEdgeAAQuadRenderStep::PerEdgeAAQuadRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("PerEdgeAAQuadRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*vertexAttrs=*/{
                            {"normal", VertexAttribType::kFloat2, SkSLType::kFloat2},
                     },
                     /*instanceAttrs=*/
                            {{"edgeFlags", VertexAttribType::kUByte4_norm, SkSLType::kFloat4},
                             {"quadXs", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"quadYs", VertexAttribType::kFloat4, SkSLType::kFloat4},

                             // TODO: pack depth and ssbo index into one 32-bit attribute, if we can
                             // go without needing both render step and paint ssbo index attributes.
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndices", VertexAttribType::kUShort2, SkSLType::kUShort2},

                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3}},
                     /*varyings=*/{
                             // Device-space distance to LTRB edges of quad.
                             {"edgeDistances", SkSLType::kFloat4}, // distance to LTRB edges
                     }) {
    // Initialize the static buffers we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. Since there should only
    // ever be one PerEdgeAAQuadRenderStep at a time, this shouldn't be an issue.
    write_vertex_buffer(bufferManager->getVertexWriter(sizeof(Vertex) * kVertexCount,
                                                       &fVertexBuffer));
    write_index_buffer(bufferManager->getIndexWriter(sizeof(uint16_t) * kIndexCount,
                                                     &fIndexBuffer));
}

PerEdgeAAQuadRenderStep::~PerEdgeAAQuadRenderStep() {}

std::string PerEdgeAAQuadRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = per_edge_aa_quad_vertex_fn("
                   // Vertex Attributes
                   "normal, "
                   // Instance Attributes
                   "edgeFlags, quadXs, quadYs, depth, "
                   "float3x3(mat0, mat1, mat2), "
                   // Varyings
                   "edgeDistances, "
                   // Render Step
                   "stepLocalCoords);\n";
}

const char* PerEdgeAAQuadRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    return "outputCoverage = per_edge_aa_quad_coverage_fn(sk_FragCoord, edgeDistances);";
}

float PerEdgeAAQuadRenderStep::boundsOutset(const Transform& localToDevice,
                                           const Rect& bounds) const {
    return local_aa_radius(localToDevice, bounds);
}

void PerEdgeAAQuadRenderStep::writeVertices(DrawWriter* writer,
                                           const DrawParams& params,
                                           skvx::ushort2 ssboIndices) const {
    SkASSERT(params.geometry().isEdgeAAQuad());
    const EdgeAAQuad& quad = params.geometry().edgeAAQuad();

    DrawWriter::Instances instance{*writer, fVertexBuffer, fIndexBuffer, kIndexCount};
    auto vw = instance.append(1);

    // Empty fills should not have been recorded at all.
    SkDEBUGCODE(Rect bounds = params.geometry().bounds());
    SkASSERT(!bounds.isEmptyNegativeOrNaN());

    constexpr uint8_t kAAOn = 255;
    constexpr uint8_t kAAOff = 0;
    auto edgeSigns = skvx::byte4{quad.edgeFlags() & AAFlags::kLeft   ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kTop    ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kRight  ? kAAOn : kAAOff,
                                 quad.edgeFlags() & AAFlags::kBottom ? kAAOn : kAAOff};

    // The vertex shader expects points to be in clockwise order. EdgeAAQuad is the only
    // shape that *might* have counter-clockwise input.
    if (is_clockwise(quad)) {
        vw << edgeSigns << quad.xs() << quad.ys();
    } else {
        vw << skvx::shuffle<2,1,0,3>(edgeSigns)  // swap left and right AA bits
           << skvx::shuffle<1,0,3,2>(quad.xs())  // swap TL with TR, and BL with BR
           << skvx::shuffle<1,0,3,2>(quad.ys()); //   ""
    }

    // All instance types share the remaining instance attribute definitions
    const SkM44& m = params.transform().matrix();

    vw << params.order().depthAsFloat()
       << ssboIndices
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void PerEdgeAAQuadRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                       PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
