/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuadPerEdgeAA.h"
#include "GrQuad.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "SkNx.h"

namespace {

// This computes the four edge equations for a quad, then outsets them and optionally computes a new
// quad as the intersection points of the outset edges. 'x' and 'y' contain the original points as
// input and the outset points as output. 'a', 'b', and 'c' are the edge equation coefficients on
// output. The values in x, y, u, v, and r are possibly updated if outsetting is needed.
// r is the local position's w component if it exists.
static void compute_quad_edges_and_outset_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y, Sk4f* a,
                                                   Sk4f* b, Sk4f* c, Sk4f* u, Sk4f* v, Sk4f* r,
                                                   int uvrChannelCount, bool outsetCorners) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);

    static constexpr auto fma = SkNx_fma<4, float>;
    // These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
    // order.
    auto nextCW  = [](const Sk4f& v) { return SkNx_shuffle<2, 0, 3, 1>(v); };
    auto nextCCW = [](const Sk4f& v) { return SkNx_shuffle<1, 3, 0, 2>(v); };

    // Compute edge equations for the quad.
    auto xnext = nextCCW(*x);
    auto ynext = nextCCW(*y);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    auto xdiff = xnext - *x;
    auto ydiff = ynext - *y;
    auto invLengths = fma(xdiff, xdiff, ydiff * ydiff).rsqrt();
    xdiff *= invLengths;
    ydiff *= invLengths;

    // Use above vectors to compute edge equations.
    *c = fma(xnext, *y,  -ynext * *x) * invLengths;
    // Make sure the edge equations have their normals facing into the quad in device space.
    auto test = fma(ydiff, nextCW(*x), fma(-xdiff, nextCW(*y), *c));
    if ((test < Sk4f(0)).anyTrue()) {
        *a = -ydiff;
        *b = xdiff;
        *c = -*c;
    } else {
        *a = ydiff;
        *b = -xdiff;
    }
    // Outset the edge equations so aa coverage evaluates to zero half a pixel away from the
    // original quad edge.
    *c += 0.5f;

    if (aaFlags != GrQuadAAFlags::kAll) {
        // This order is the same order the edges appear in xdiff/ydiff and therefore as the
        // edges in a/b/c.
        auto mask = Sk4f(GrQuadAAFlags::kLeft & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kTop & aaFlags ? 1.f : 0.f,
                         GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        // Outset edge equations for masked out edges another pixel so that they always evaluate
        // >= 1.
        *c += (1.f - mask);
        if (outsetCorners) {
            // Do the vertex outset.
            mask *= 0.5f;
            auto maskCW = nextCW(mask);
            *x += maskCW * -xdiff + mask * nextCW(xdiff);
            *y += maskCW * -ydiff + mask * nextCW(ydiff);
            if (uvrChannelCount > 0) {
                // We want to extend the texture coords by the same proportion as the positions.
                maskCW *= invLengths;
                mask *= nextCW(invLengths);
                Sk4f udiff = nextCCW(*u) - *u;
                Sk4f vdiff = nextCCW(*v) - *v;
                *u += maskCW * -udiff + mask * nextCW(udiff);
                *v += maskCW * -vdiff + mask * nextCW(vdiff);
                if (uvrChannelCount == 3) {
                    Sk4f rdiff = nextCCW(*r) - *r;
                    *r += maskCW * -rdiff + mask * nextCW(rdiff);
                }
            }
        }
    } else if (outsetCorners) {
        *x += 0.5f * (-xdiff + nextCW(xdiff));
        *y += 0.5f * (-ydiff + nextCW(ydiff));
        if (uvrChannelCount > 0) {
            Sk4f t = 0.5f * invLengths;
            Sk4f udiff = nextCCW(*u) - *u;
            Sk4f vdiff = nextCCW(*v) - *v;
            *u += t * -udiff + nextCW(t) * nextCW(udiff);
            *v += t * -vdiff + nextCW(t) * nextCW(vdiff);
            if (uvrChannelCount == 3) {
                Sk4f rdiff = nextCCW(*r) - *r;
                *r += t * -rdiff + nextCW(t) * nextCW(rdiff);
            }
        }
    }
}

// Generalizes the above function to extrapolate local coords such that after perspective division
// of the device coordinate, the original local coordinate value is at the original un-outset
// device position. r is the local coordinate's w component.
static void compute_quad_edges_and_outset_persp_vertices(GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
                                                         Sk4f* w, Sk4f* a, Sk4f* b, Sk4f* c,
                                                         Sk4f* u, Sk4f* v, Sk4f* r,
                                                         int uvrChannelCount) {
    SkASSERT(uvrChannelCount == 0 || uvrChannelCount == 2 || uvrChannelCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;
    // Don't compute outset corners in the normalized space, which means u, v, and r don't need
    // to be provided here (outset separately below).
    compute_quad_edges_and_outset_vertices(aaFlags, &x2d, &y2d, a, b, c, nullptr, nullptr, nullptr,
                                           /* uvr ct */ 0, /* outsetCorners */ false);

    static const float kOutset = 0.5f;
    if ((GrQuadAAFlags::kLeft | GrQuadAAFlags::kRight) & aaFlags) {
        // For each entry in x the equivalent entry in opX is the left/right opposite and so on.
        Sk4f opX = SkNx_shuffle<2, 3, 0, 1>(*x);
        Sk4f opW = SkNx_shuffle<2, 3, 0, 1>(*w);
        Sk4f opY = SkNx_shuffle<2, 3, 0, 1>(*y);
        // vx/vy holds the device space left-to-right vectors along top and bottom of the quad.
        Sk2f vx = SkNx_shuffle<2, 3>(x2d) - SkNx_shuffle<0, 1>(x2d);
        Sk2f vy = SkNx_shuffle<2, 3>(y2d) - SkNx_shuffle<0, 1>(y2d);
        Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();
        // For each device space corner, devP, label its left/right opposite device space point
        // opDevPt. The new device space point is opDevPt + s (devPt - opDevPt) where s is
        // (length(devPt - opDevPt) + 0.5) / length(devPt - opDevPt);
        Sk4f s = SkNx_shuffle<0, 1, 0, 1>((len + kOutset) / len);
        // Compute t in homogeneous space from s using similar triangles so that we can produce
        // homogeneous outset vertices for perspective-correct interpolation.
        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));
        // mask is used to make the t values be 1 when the left/right side is not antialiased.
        Sk4f mask(GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kLeft & aaFlags  ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kRight & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrChannelCount > 0) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrChannelCount == 3) {
                Sk4f opR = SkNx_shuffle<2, 3, 0, 1>(*r);
                *r = opR + t * (*r - opR);
            }
        }

        if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
            // Update the 2D points for the top/bottom calculation.
            iw = (*w).invert();
            x2d = (*x) * iw;
            y2d = (*y) * iw;
        }
    }

    if ((GrQuadAAFlags::kTop | GrQuadAAFlags::kBottom) & aaFlags) {
        // This operates the same as above but for top/bottom rather than left/right.
        Sk4f opX = SkNx_shuffle<1, 0, 3, 2>(*x);
        Sk4f opW = SkNx_shuffle<1, 0, 3, 2>(*w);
        Sk4f opY = SkNx_shuffle<1, 0, 3, 2>(*y);

        Sk2f vx = SkNx_shuffle<1, 3>(x2d) - SkNx_shuffle<0, 2>(x2d);
        Sk2f vy = SkNx_shuffle<1, 3>(y2d) - SkNx_shuffle<0, 2>(y2d);
        Sk2f len = SkNx_fma(vx, vx, vy * vy).sqrt();

        Sk4f s = SkNx_shuffle<0, 0, 1, 1>((len + kOutset) / len);

        Sk4f sOpW = s * opW;
        Sk4f t = sOpW / (sOpW + (1.f - s) * (*w));

        Sk4f mask(GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kTop    & aaFlags ? 1.f : 0.f,
                  GrQuadAAFlags::kBottom & aaFlags ? 1.f : 0.f);
        t = t * mask + (1.f - mask);
        *x = opX + t * (*x - opX);
        *y = opY + t * (*y - opY);
        *w = opW + t * (*w - opW);

        if (uvrChannelCount > 0) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrChannelCount == 3) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }
}

// Fast path for non-AA quads batched into an AA op. Since they are part of the AA op, the vertices
// need to have valid edge equations that ensure coverage is set to 1. To get perspective
// interpolation of the edge distance, the vertex shader outputs d*w and then multiplies by 1/w in
// the fragment shader. For non-AA edges, the edge equation can be simplified to 0*x/w + y/w + c >=
// 1, so the vertex shader outputs c*w. The quad is sent as two triangles, so a fragment is the
// interpolation between 3 of the 4 vertices. If iX are the weights for the 3 involved quad
// vertices, then the fragment shader's state is:
//   f_cw = c * (iA*wA + iB*wB + iC*wC) and f_1/w = iA/wA + iB/wB + iC/wC
//   (where A,B,C are chosen from {1,2,3, 4})
// When there's no perspective, then f_cw*f_1/w = c and setting c = 1 guarantees a proper non-AA
// edge. Unfortunately when there is perspective, f_cw*f_1/w != c unless the fragment is at a
// vertex. We must pick a c such that f_cw*f_1/w >= 1 across the whole primitive.
// Let n = min(w1,w2,w3,w4) and m = max(w1,w2,w3,w4) and rewrite
//   f_1/w=(iA*wB*wC + iB*wA*wC + iC*wA*wB) / (wA*wB*wC)
// Since the iXs are weights for the interior of the primitive, then we have:
//   n <= (iA*wA + iB*wB + iC*wC) <= m and
//   n^2 <= (iA*wB*wC + iB*wA*wC + iC*wA*wB) <= m^2 and
//   n^3 <= wA*wB*wC <= m^3 regardless of the choice of A,B, and C
// Thus if we set c = m^3/n^3, it guarantees f_cw*f_1/w >= 1 for any perspective.
static SkPoint3 compute_non_aa_persp_edge_coeffs(const Sk4f& w) {
    float n = w.min();
    float m = w.max();
    return {0.f, 0.f, (m * m * m) / (n * n * n)};
}

// When there's guaranteed no perspective, the edge coefficients for non-AA quads is constant
static constexpr SkPoint3 kNonAANoPerspEdgeCoeffs = {0, 0, 1};

// This packs the four quad vertices' values for a given channel (the data) into a block. Returns
// the offset for the next block to be written to localStorage
static int store(const Sk4f& data, float* localStorage, int offset) {
    data.store(localStorage + offset);
    return offset + 4;
}

// This unpacks dimCt values from a series of channels. By initializing offset from 0 to 3 (plus
// any channels' offsets to skip over), the particular quad vertex can be accessed. Returns the
// offset for the next channel of data in localStorage.
static int load(const float* localStorage, int offset, float* coordOut, int dimCt) {
    for (int i = 0; i < dimCt; i++) {
        coordOut[i] = localStorage[offset];
        offset += 4;
    }
    return offset;
}

} // anonymous namespace

void GrQuadPerEdgeAA::TessellateImpl(void* vertices, size_t vertexSize, float* localStorage,
        const GrPerspQuad& deviceQuad, int posDim, size_t posOffset, size_t posSize,
        const void* color, size_t colorOffset, size_t colorSize,
        const GrPerspQuad& srcQuad, int srcDim, size_t srcOffset, size_t srcSize,
        const void* domain, size_t domainOffset, size_t domainSize,
        GrQuadAAFlags aaFlags, size_t aaOffset, size_t aaSize) {
    // Make sure the device and local positions are dimensions that are supported
    SkASSERT(posDim == 2 || posDim == 3);
    SkASSERT(srcDim == 0 || srcDim == 2 || srcDim == 3);
    // Make sure that the position sizes are the proper multiples of sizeof(float) since we copy
    // floats directly into the block without converting types
    SkASSERT(posSize == posDim * sizeof(float));
    SkASSERT(srcSize == srcDim * sizeof(float));
    // Make sure the component sizes completely fill the vertex
    SkASSERT(vertexSize == posSize + colorSize + srcSize + domainSize + aaSize);

    // Load position data into Sk4fs (always x, y and maybe w)
    Sk4f x = deviceQuad.x4f();
    Sk4f y = deviceQuad.y4f();
    Sk4f w;
    if (posDim == 3) {
        w = deviceQuad.w4f();
    }

    // Load local position data into Sk4fs (either none, just u,v or all three)
    Sk4f u, v, r;
    if (srcDim > 0) {
        u = srcQuad.x4f();
        v = srcQuad.y4f();

        if (srcDim == 3) {
            r = srcQuad.w4f();
        }
    }

    Sk4f a, b, c;
    if (aaSize) {
        // Must calculate edges and possibly outside the positions
        if (aaFlags == GrQuadAAFlags::kNone) {
            // A non-AA quad that got batched into an AA group, so its edges will be the same for
            // all four vertices and it does not need to be outset
            SkPoint3 edgeCoeffs;
            if (posDim == 3) {
                edgeCoeffs = compute_non_aa_persp_edge_coeffs(w);
            } else {
                edgeCoeffs = kNonAANoPerspEdgeCoeffs;
            }

            // Copy the coefficients into all four equations
            a = edgeCoeffs.fX;
            b = edgeCoeffs.fY;
            c = edgeCoeffs.fZ;
        } else if (posDim == 2) {
            // For simplicity, pointers to u, v, and r are always provided, but srcDim
            // ensures that only loaded Sk4fs are modified in the compute functions.
            compute_quad_edges_and_outset_vertices(
                    aaFlags, &x, &y, &a, &b, &c, &u, &v, &r, srcDim, /* outset */ true);
        } else {
            compute_quad_edges_and_outset_persp_vertices(
                    aaFlags, &x, &y, &w, &a, &b, &c, &u, &v, &r, srcDim);
        }
    }

    // It is faster to unpack the Sk4fs all at once than access their components out of order.
    int offset = store(x, localStorage, 0);
    offset = store(y, localStorage, offset);
    if (posDim == 3) {
        offset = store(w, localStorage, offset);
    }
    if (srcDim > 0) {
        offset = store(u, localStorage, offset);
        offset = store(v, localStorage, offset);
        if (srcDim == 3) {
            offset = store(w, localStorage, offset);
        }
    }
    int edgeOffset = offset; // The 4 edges are separate from the 4 vertices
    if (aaSize) {
        offset = store(a, localStorage, offset);
        offset = store(b, localStorage, offset);
        offset = store(c, localStorage, offset);
    }
    // Now rearrange the unpacked buffer into the vertex layout
    char* vb = reinterpret_cast<char*>(vertices);
    for (int i = 0; i < 4; ++i) {
        // Starting the offset at i makes sure that all loads read the data for the i^th vertex
        offset = i;

        // NOTE: while this code uses explicit offsets to make it independent of the actual
        // vertex layout, it is a good idea to keep the writes in the same order as the fields

        // save position
        offset = load(localStorage, offset, reinterpret_cast<float*>(vb + posOffset), posDim);
        // save color
        if (colorSize) {
            memcpy(vb + colorOffset, color, colorSize);
        }
        // save local position
        if (srcDim) {
            offset = load(localStorage, offset, reinterpret_cast<float*>(vb + srcOffset), srcDim);
        }
        // save the domain
        if (domainSize) {
            memcpy(vb + domainOffset, domain, domainSize);
        }

        // save the edges
        if (aaSize) {
            float* edgeBuffer = reinterpret_cast<float*>(vb + aaOffset);
            for (int j = 0; j < 4; j++) {
                load(localStorage, edgeOffset + j, edgeBuffer, 3);
                edgeBuffer += 3;
            }
        }

        vb += vertexSize;
    }
}

GrQuadPerEdgeAA::GPAttributes::GPAttributes(int posDim, int localDim, bool hasColor, GrAAType aa,
                                            Domain domain) {
    SkASSERT(posDim == 2 || posDim == 3);
    SkASSERT(localDim == 0 || localDim == 2 || localDim == 3);

    if (posDim == 3) {
        fPositions = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    } else {
        fPositions = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    }

    if (localDim == 3) {
        fLocalCoords = {"localCoord", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    } else if (localDim == 2) {
        fLocalCoords = {"localCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    } // else localDim == 0 and attribute remains uninitialized

    if (hasColor) {
        fColors = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
    }

    if (domain == Domain::kYes) {
        fDomain = {"domain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
    }

    if (aa == GrAAType::kCoverage) {
        fAAEdges[0] = {"aaEdge0", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        fAAEdges[1] = {"aaEdge1", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        fAAEdges[2] = {"aaEdge2", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        fAAEdges[3] = {"aaEdge3", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
    }
}

bool GrQuadPerEdgeAA::GPAttributes::needsPerspectiveInterpolation() const {
    // This only needs to check the position; local position having perspective does not change
    // how the varyings are interpolated
    return fPositions.cpuType() == kFloat3_GrVertexAttribType;
}

int GrQuadPerEdgeAA::GPAttributes::vertexAttributeCount() const {
    // Always has position, hence 1+
    return (1 + this->hasLocalCoords() + this->hasVertexColors() + this->hasDomain() +
            4 * this->usesCoverageAA());
}

uint32_t GrQuadPerEdgeAA::GPAttributes::getKey() const {
    // aa, color, domain are single bit flags
    uint32_t x = this->usesCoverageAA() ? 0 : 1;
    x |= this->hasVertexColors() ? 0 : 2;
    x |= this->hasDomain() ? 0 : 4;
    // regular position has two options as well
    x |= kFloat3_GrVertexAttribType == fPositions.cpuType() ? 0 : 8;
    // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
    if (this->hasLocalCoords()) {
        x |= kFloat3_GrVertexAttribType == fLocalCoords.cpuType() ? 16 : 32;
    }
    return x;
}

void GrQuadPerEdgeAA::GPAttributes::emitColor(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                              GrGLSLColorSpaceXformHelper* csXformHelper,
                                              const char* colorVarName) const {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    if (!fColors.isInitialized()) {
        return;
    }

    if (csXformHelper == nullptr || csXformHelper->isNoop()) {
        args.fVaryingHandler->addPassThroughAttribute(
                fColors, args.fOutputColor, Interpolation::kCanBeFlat);
    } else {
        GrGLSLVarying varying(kHalf4_GrSLType);
        args.fVaryingHandler->addVarying(colorVarName, &varying);
        args.fVertBuilder->codeAppendf("half4 %s = ", colorVarName);
        args.fVertBuilder->appendColorGamutXform(fColors.name(), csXformHelper);
        args.fVertBuilder->codeAppend(";");
        args.fVertBuilder->codeAppendf("%s = half4(%s.rgb * %s.a, %s.a);",
                                       varying.vsOut(), colorVarName, colorVarName, colorVarName);
        args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor, varying.fsIn());
    }
}

void GrQuadPerEdgeAA::GPAttributes::emitExplicitLocalCoords(
        GrGLSLPrimitiveProcessor::EmitArgs& args, const char* localCoordName,
        const char* domainName) const {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;
    if (!this->hasLocalCoords()) {
        return;
    }

    args.fFragBuilder->codeAppendf("float2 %s;", localCoordName);
    bool localHasPerspective = fLocalCoords.cpuType() == kFloat3_GrVertexAttribType;
    if (localHasPerspective) {
        // Can't do a pass through since we need to perform perspective division
        GrGLSLVarying v(fLocalCoords.gpuType());
        args.fVaryingHandler->addVarying(fLocalCoords.name(), &v);
        args.fVertBuilder->codeAppendf("%s = %s;", v.vsOut(), fLocalCoords.name());
        args.fFragBuilder->codeAppendf("%s = %s.xy / %s.z;", localCoordName, v.fsIn(), v.fsIn());
    } else {
        args.fVaryingHandler->addPassThroughAttribute(fLocalCoords, localCoordName);
    }

    // Clamp the now 2D localCoordName variable by the domain if it is provided
    if (this->hasDomain()) {
        args.fFragBuilder->codeAppendf("float4 %s;", domainName);
        args.fVaryingHandler->addPassThroughAttribute(fDomain, domainName,
                                                      Interpolation::kCanBeFlat);
        args.fFragBuilder->codeAppendf("%s = clamp(%s, %s.xy, %s.zw);",
                                       localCoordName, localCoordName, domainName, domainName);
    }
}

void GrQuadPerEdgeAA::GPAttributes::emitCoverage(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                                 const char* edgeDistName) const {
    if (this->usesCoverageAA()) {
        bool mulByFragCoordW = false;
        GrGLSLVarying aaDistVarying(kFloat4_GrSLType,
                                    GrGLSLVarying::Scope::kVertToFrag);
        args.fVaryingHandler->addVarying(edgeDistName, &aaDistVarying);

        if (kFloat3_GrVertexAttribType == fPositions.cpuType()) {
            // The distance from edge equation e to homogeneous point p=sk_Position
            // is e.x*p.x/p.w + e.y*p.y/p.w + e.z. However, we want screen space
            // interpolation of this distance. We can do this by multiplying the
            // varying in the VS by p.w and then multiplying by sk_FragCoord.w in
            // the FS. So we output e.x*p.x + e.y*p.y + e.z * p.w
            args.fVertBuilder->codeAppendf(
                    R"(%s = float4(dot(%s, %s), dot(%s, %s),
                                   dot(%s, %s), dot(%s, %s));)",
                    aaDistVarying.vsOut(), fAAEdges[0].name(), fPositions.name(),
                    fAAEdges[1].name(), fPositions.name(), fAAEdges[2].name(), fPositions.name(),
                    fAAEdges[3].name(), fPositions.name());
            mulByFragCoordW = true;
        } else {
            args.fVertBuilder->codeAppendf(
                    R"(%s = float4(dot(%s.xy, %s.xy) + %s.z,
                                   dot(%s.xy, %s.xy) + %s.z,
                                   dot(%s.xy, %s.xy) + %s.z,
                                   dot(%s.xy, %s.xy) + %s.z);)",
                    aaDistVarying.vsOut(),
                    fAAEdges[0].name(), fPositions.name(), fAAEdges[0].name(),
                    fAAEdges[1].name(), fPositions.name(), fAAEdges[1].name(),
                    fAAEdges[2].name(), fPositions.name(), fAAEdges[2].name(),
                    fAAEdges[3].name(), fPositions.name(), fAAEdges[3].name());
        }

        args.fFragBuilder->codeAppendf(
                "float min%s = min(min(%s.x, %s.y), min(%s.z, %s.w));",
                edgeDistName, aaDistVarying.fsIn(), aaDistVarying.fsIn(), aaDistVarying.fsIn(),
                aaDistVarying.fsIn());
        if (mulByFragCoordW) {
            args.fFragBuilder->codeAppendf("min%s *= sk_FragCoord.w;", edgeDistName);
        }
        args.fFragBuilder->codeAppendf("%s = float4(saturate(min%s));",
                                       args.fOutputCoverage, edgeDistName);
    } else {
        // Set coverage to 1
        args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
    }
}
