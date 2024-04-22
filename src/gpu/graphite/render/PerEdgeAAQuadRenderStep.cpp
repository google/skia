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

    if (writer) {
        writer << kIndices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
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

    if (writer) {
        writer << kCornerTemplate  // TL
               << kCornerTemplate  // TR
               << kCornerTemplate  // BR
               << kCornerTemplate; // BL
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

PerEdgeAAQuadRenderStep::PerEdgeAAQuadRenderStep(StaticBufferManager* bufferManager)
        : RenderStep("PerEdgeAAQuadRenderStep",
                     "",
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kOutsetBoundsForAA,
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
