/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/render/CircularArcRenderStep.h"

#include "include/core/SkArc.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/DrawWriter.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/render/CommonDepthStencilSettings.h"

#include <utility>

// This RenderStep is used to render filled circular arcs and stroked circular arcs that
// don't include the center. Currently it only supports butt caps but will be extended
// to include round caps.
//
// Each arc is represented by a single instance. The instance attributes are enough to
// describe the given arc types without relying on uniforms to define its operation.
// The attributes encode shape as follows:

// float4 centerScales - used to transform the vertex data into local space.
//    The vertex data represents interleaved octagons that are respectively circumscribed
//    and inscribed on a unit circle, and have to be transformed into local space.
//    So the .xy values here are the center of the arc in local space, and .zw its outer and inner
//    radii, respectively. If the vertex is an outer vertex its local position will be computed as
//         centerScales.xy + position.xy * centerScales.z
//    Otherwise it will be computed as
//         centerScales.xy + position.xy * centerScales.w
//    We can tell whether a vertex is an outer or inner vertex by looking at the sign
//    of its z component. This z value is also used to compute half-pixel anti-aliasing offsets
//    once the vertex data is transformed into device space.
// float3 radiiAndFlags - in the fragment shader we will pass an offset in unit circle space to
//    determine the circle edge and for use for clipping. The .x value here is outerRadius+0.5 and
//    will be compared against the unit circle radius (i.e., 1.0) to compute the outer edge. The .y
//    value is innerRadius-0.5/outerRadius+0.5 and will be used as the comparison point for the
//    inner edge. The .z value is a flag which indicates whether fragClipPlane1 is for intersection
//    (+) or for union (-), and whether to set up rounded caps (-2/+2).
// float3 geoClipPlane - For very thin acute arcs, because of the 1/2 pixel boundary we can get
//    non-clipped artifacts beyond the center of the circle. To solve this, we clip the geometry
//    so any rendering doesn't cross that point.

// In addition, these values will be passed to the fragment shader:
//
// float3 fragClipPlane0 - the arc will always be clipped against this half plane, and passed as
//    the varying clipPlane.
// float3 fragClipPlane1 - for convex/acute arcs, we pass this via the varying isectPlane to clip
//    against this and multiply its value by the ClipPlane clip result. For concave/obtuse arcs,
//    we pass this via the varying unionPlane which will clip against this and add its value to the
//    ClipPlane clip result. This is controlled by the flag value in radiiAndFlags: if the
//    flag is > 0, it's passed as isectClip, if it's < 0 it's passed as unionClip. We set default
//    values for the alternative clip plane that end up being a null clip.
// float  roundCapRadius - this is computed in the vertex shader. If we're using round caps (i.e.,
//    if abs(flags) > 1), this will be half the distance between the outer and inner radii.
//    Otherwise it will be 0 which will end up zeroing out any round cap calculation.
// float4 inRoundCapPos - locations of the centers of the round caps in normalized space. This
//    will be all zeroes if not needed.

namespace skgpu::graphite {

// Represents the per-vertex attributes used in each instance.
struct Vertex {
    // Unit circle local space position (.xy) and AA offset (.z)
    SkV3 fPosition;
};

static constexpr int kVertexCount = 18;

static void write_vertex_buffer(VertexWriter writer) {
    // Normalized geometry for octagons that circumscribe/inscribe a unit circle.
    // Outer ring offset
    static constexpr float kOctOffset = 0.41421356237f;  // sqrt(2) - 1
    // Inner ring points
    static constexpr SkScalar kCosPi8 = 0.923579533f;
    static constexpr SkScalar kSinPi8 = 0.382683432f;

    // Directional offset for anti-aliasing.
    // Also used as marker for whether this is an outer or inner vertex.
    static constexpr float kOuterAAOffset = 0.5f;
    static constexpr float kInnerAAOffset = -0.5f;

    static constexpr SkV3 kOctagonVertices[kVertexCount] = {
        {-kOctOffset, -1,          kOuterAAOffset},
        {-kSinPi8,    -kCosPi8,    kInnerAAOffset},
        { kOctOffset, -1,          kOuterAAOffset},
        {kSinPi8,     -kCosPi8,    kInnerAAOffset},
        { 1,          -kOctOffset, kOuterAAOffset},
        {kCosPi8,     -kSinPi8,    kInnerAAOffset},
        { 1,           kOctOffset, kOuterAAOffset},
        {kCosPi8,      kSinPi8,    kInnerAAOffset},
        { kOctOffset,  1,          kOuterAAOffset},
        {kSinPi8,      kCosPi8,    kInnerAAOffset},
        {-kOctOffset,  1,          kOuterAAOffset},
        {-kSinPi8,     kCosPi8,    kInnerAAOffset},
        {-1,           kOctOffset, kOuterAAOffset},
        {-kCosPi8,     kSinPi8,    kInnerAAOffset},
        {-1,          -kOctOffset, kOuterAAOffset},
        {-kCosPi8,    -kSinPi8,    kInnerAAOffset},
        {-kOctOffset, -1,          kOuterAAOffset},
        {-kSinPi8,    -kCosPi8,    kInnerAAOffset},
    };

    if (writer) {
        writer << kOctagonVertices;
    } // otherwise static buffer creation failed, so do nothing; Context initialization will fail.
}

CircularArcRenderStep::CircularArcRenderStep(StaticBufferManager* bufferManager)
        : RenderStep(RenderStepID::kCircularArc,
                     Flags::kPerformsShading | Flags::kEmitsCoverage | Flags::kOutsetBoundsForAA |
                     Flags::kAppendInstances,
                     /*uniforms=*/{},
                     PrimitiveType::kTriangleStrip,
                     kDirectDepthGreaterPass,
                     /*staticAttrs=*/{
                             {"position", VertexAttribType::kFloat3, SkSLType::kFloat3},
                     },
                     /*appendAttrs=*/{
                             // Center plus radii, used to transform to local position
                             {"centerScales", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             // Outer (device space) and inner (normalized) radii
                             // + flags for determining clipping and roundcaps
                             {"radiiAndFlags", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             // Clips the geometry for acute arcs
                             {"geoClipPlane", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             // Clip planes sent to the fragment shader for arc extents
                             {"fragClipPlane0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"fragClipPlane1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             // Roundcap positions, if needed
                             {"inRoundCapPos", VertexAttribType::kFloat4, SkSLType::kFloat4},
                             {"inRoundCapRadius", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"depth", VertexAttribType::kFloat, SkSLType::kFloat},
                             {"ssboIndices", VertexAttribType::kUInt2, SkSLType::kUInt2},

                             {"mat0", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat1", VertexAttribType::kFloat3, SkSLType::kFloat3},
                             {"mat2", VertexAttribType::kFloat3, SkSLType::kFloat3},
                     },
                     /*varyings=*/{
                             // Normalized offset vector plus radii
                             {"circleEdge", SkSLType::kFloat4},
                             // Half-planes used to clip to arc shape.
                             {"clipPlane", SkSLType::kFloat3},
                             {"isectPlane", SkSLType::kFloat3},
                             {"unionPlane", SkSLType::kFloat3},
                             // Roundcap data
                             {"roundCapRadius", SkSLType::kFloat},
                             {"roundCapPos", SkSLType::kFloat4},
                     }) {
    // Initialize the static buffer we'll use when recording draw calls.
    // NOTE: Each instance of this RenderStep gets its own copy of the data. Since there should only
    // ever be one CircularArcRenderStep at a time, this shouldn't be an issue.
    write_vertex_buffer(bufferManager->getVertexWriter(kVertexCount, sizeof(Vertex),
                                                       &fVertexBuffer));
}

CircularArcRenderStep::~CircularArcRenderStep() {}

std::string CircularArcRenderStep::vertexSkSL() const {
    // Returns the body of a vertex function, which must define a float4 devPosition variable and
    // must write to an already-defined float2 stepLocalCoords variable.
    return "float4 devPosition = circular_arc_vertex_fn("
                   // Static Data Attributes
                   "position, "
                   // Append Data Attributes
                   "centerScales, radiiAndFlags, geoClipPlane, fragClipPlane0, fragClipPlane1, "
                   "inRoundCapPos, inRoundCapRadius, depth, float3x3(mat0, mat1, mat2), "
                   // Varyings
                   "circleEdge, clipPlane, isectPlane, unionPlane, "
                   "roundCapRadius, roundCapPos, "
                   // Render Step
                   "stepLocalCoords);\n";
}

const char* CircularArcRenderStep::fragmentCoverageSkSL() const {
    // The returned SkSL must write its coverage into a 'half4 outputCoverage' variable (defined in
    // the calling code) with the actual coverage splatted out into all four channels.
    return "outputCoverage = circular_arc_coverage_fn(circleEdge, "
                                                     "clipPlane, "
                                                     "isectPlane, "
                                                     "unionPlane, "
                                                     "roundCapRadius, "
                                                     "roundCapPos);";
}

void CircularArcRenderStep::writeVertices(DrawWriter* writer,
                                          const DrawParams& params,
                                          skvx::uint2 ssboIndices) const {
    SkASSERT(params.geometry().isShape() && params.geometry().shape().isArc());

    DrawWriter::Instances instance{*writer, fVertexBuffer, {}, kVertexCount};
    auto vw = instance.append(1);

    const Shape& shape = params.geometry().shape();
    const SkArc& arc = shape.arc();

    SkPoint localCenter = arc.oval().center();
    float localOuterRadius = arc.oval().width() / 2;
    float localInnerRadius = 0.0f;

    // We know that we have a similarity matrix so this will transform radius to device space
    const Transform& transform = params.transform();
    float radius = localOuterRadius * transform.maxScaleFactor();
    bool isStroke = params.isStroke();

    float innerRadius = -SK_ScalarHalf;
    float outerRadius = radius;
    float halfWidth = 0;
    if (isStroke) {
        float localHalfWidth = params.strokeStyle().halfWidth();

        halfWidth = localHalfWidth * transform.maxScaleFactor();
        if (SkScalarNearlyZero(halfWidth)) {
            halfWidth = SK_ScalarHalf;
            // Need to map this back to local space
            localHalfWidth = halfWidth / transform.maxScaleFactor();
        }

        outerRadius += halfWidth;
        innerRadius = radius - halfWidth;
        localInnerRadius = localOuterRadius - localHalfWidth;
        localOuterRadius += localHalfWidth;
    }

    // The radii are outset for two reasons. First, it allows the shader to simply perform
    // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
    // Second, the outer radius is used to compute the verts of the bounding box that is
    // rendered and the outset ensures the box will cover all partially covered by the circle.
    outerRadius += SK_ScalarHalf;
    innerRadius -= SK_ScalarHalf;

    // The shader operates in a space where the circle is translated to be centered at the
    // origin. Here we compute points on the unit circle at the starting and ending angles.
    SkV2 localPoints[3];
    float startAngleRadians = SkDegreesToRadians(arc.startAngle());
    float sweepAngleRadians = SkDegreesToRadians(arc.sweepAngle());
    localPoints[0].y = SkScalarSin(startAngleRadians);
    localPoints[0].x = SkScalarCos(startAngleRadians);
    SkScalar endAngle = startAngleRadians + sweepAngleRadians;
    localPoints[1].y = SkScalarSin(endAngle);
    localPoints[1].x = SkScalarCos(endAngle);
    localPoints[2] = {0, 0};

    // Adjust the start and end points based on the view matrix (to handle rotated arcs)
    SkV4 devPoints[3];
    transform.mapPoints(localPoints, devPoints, 3);
    // Translate the point relative to the transformed origin
    SkV2 startPoint = {devPoints[0].x - devPoints[2].x, devPoints[0].y - devPoints[2].y};
    SkV2 stopPoint = {devPoints[1].x - devPoints[2].x, devPoints[1].y - devPoints[2].y};
    startPoint = startPoint.normalize();
    stopPoint = stopPoint.normalize();

    // We know the matrix is a similarity here. Detect mirroring which will affect how we
    // should orient the clip planes for arcs.
    const SkM44& m = transform.matrix();
    auto upperLeftDet = m.rc(0,0) * m.rc(1,1) -
                        m.rc(0,1) * m.rc(1,0);
    if (upperLeftDet < 0) {
        std::swap(startPoint, stopPoint);
    }

    // Like a fill without useCenter, butt-cap stroke can be implemented by clipping against
    // radial lines. We treat round caps the same way, but track coverage of circles at the
    // center of the butts.
    // However, in both cases we have to be careful about the half-circle.
    // case. In that case the two radial lines are equal and so that edge gets clipped
    // twice. Since the shared edge goes through the center we fall back on the !useCenter
    // case.
    auto absSweep = SkScalarAbs(sweepAngleRadians);
    bool useCenter = (arc.isWedge() || isStroke) &&
                     !SkScalarNearlyEqual(absSweep, SK_ScalarPI);

    // This makes every point fully inside the plane.
    SkV3 geoClipPlane = {0.f, 0.f, 1.f};
    SkV3 clipPlane0;
    SkV3 clipPlane1;
    SkV2 roundCapPos0 = {0, 0};
    SkV2 roundCapPos1 = {0, 0};
    static constexpr float kIntersection_NoRoundCaps = 1;
    static constexpr float kIntersection_RoundCaps = 2;

    float roundCapRadius = 0;
    // Default to intersection and no round caps.
    float flags = kIntersection_NoRoundCaps;
    // Determine if we need round caps.
    if (isStroke &&
        params.strokeStyle().halfWidth() > 0 &&
        params.strokeStyle().cap() == SkPaint::kRound_Cap) {
        // Compute the cap center points in the normalized space.
        float midRadius = (innerRadius + outerRadius) / (2 * outerRadius);
        roundCapPos0 = startPoint * midRadius;
        roundCapPos1 = stopPoint * midRadius;
        flags = kIntersection_RoundCaps;
        // Compute the cap radius in the normalized space.
        roundCapRadius = (outerRadius - innerRadius) / (2 * outerRadius);
    }

    // Determine clip planes.
    if (useCenter) {
        SkV2 norm0 = {startPoint.y, -startPoint.x};
        SkV2 norm1 = {stopPoint.y, -stopPoint.x};
        // This ensures that norm0 is always the clockwise plane, and norm1 is CCW.
        if (sweepAngleRadians < 0) {
            std::swap(norm0, norm1);
        }
        norm0 = -norm0;
        clipPlane0 = {norm0.x, norm0.y, 0.5f};
        clipPlane1 = {norm1.x, norm1.y, 0.5f};
        if (absSweep > SK_ScalarPI) {
            // Union
            flags = -flags;
        } else {
            // Intersection
            // Highly acute arc. We need to clip the vertices to the perpendicular half-plane.
            if (!isStroke && absSweep < 0.5f*SK_ScalarPI) {
                // We do this clipping in normalized space so use our original local points.
                // Should already be normalized since they're sin/cos.
                SkV2 localNorm0 = {localPoints[0].y, -localPoints[0].x};
                SkV2 localNorm1 = {localPoints[1].y, -localPoints[1].x};
                // This ensures that norm0 is always the clockwise plane, and norm1 is CCW.
                if (sweepAngleRadians < 0) {
                    std::swap(localNorm0, localNorm1);
                }
                // Negate norm0 and compute the perpendicular of the difference
                SkV2 clipNorm = {-localNorm0.y - localNorm1.y, localNorm1.x + localNorm0.x};
                clipNorm = clipNorm.normalize();
                // This should give us 1/2 pixel spacing from the half-plane
                // after transforming from normalized to local to device space.
                float dist = 0.5f / radius / transform.maxScaleFactor();
                geoClipPlane = {clipNorm.x, clipNorm.y, dist};
            }
        }
    } else {
        // We clip to a secant of the original circle, only one clip plane
        startPoint *= radius;
        stopPoint *= radius;
        SkV2 norm = {startPoint.y - stopPoint.y, stopPoint.x - startPoint.x};
        norm = norm.normalize();
        if (sweepAngleRadians > 0) {
            norm = -norm;
        }
        float d = -norm.dot(startPoint) + 0.5f;
        clipPlane0 = {norm.x, norm.y, d};
        clipPlane1 = {0.f, 0.f, 1.f}; // no clipping
    }

    if (isStroke && innerRadius < -SK_ScalarHalf) {
        // Reset the inner radius to render a filled arc instead of a stroked arc, as the stroke
        // width is greater than or equal to the oval's width.
        innerRadius = -SK_ScalarHalf;
        localInnerRadius = 0.f;
    }

    // The inner radius in the vertex data must be specified in normalized space.
    innerRadius = innerRadius / outerRadius;

    vw << localCenter << localOuterRadius << localInnerRadius
       << outerRadius << innerRadius << flags
       << geoClipPlane << clipPlane0 << clipPlane1
       << roundCapPos0 << roundCapPos1 << roundCapRadius
       << params.order().depthAsFloat()
       << ssboIndices
       << m.rc(0,0) << m.rc(1,0) << m.rc(3,0)  // mat0
       << m.rc(0,1) << m.rc(1,1) << m.rc(3,1)  // mat1
       << m.rc(0,3) << m.rc(1,3) << m.rc(3,3); // mat2
}

void CircularArcRenderStep::writeUniformsAndTextures(const DrawParams&,
                                                     PipelineDataGatherer*) const {
    // All data is uploaded as instance attributes, so no uniforms are needed.
}

}  // namespace skgpu::graphite
