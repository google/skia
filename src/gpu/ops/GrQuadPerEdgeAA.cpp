/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrQuadPerEdgeAA.h"
#include "GrQuad.h"
#include "GrVertexWriter.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "SkNx.h"

#define AI SK_ALWAYS_INLINE

namespace {

static AI Sk4f fma(const Sk4f& f, const Sk4f& m, const Sk4f& a) {
    return SkNx_fma<4, float>(f, m, a);
}

// These rotate the points/edge values either clockwise or counterclockwise assuming tri strip
// order.
static AI Sk4f nextCW(const Sk4f& v) {
    return SkNx_shuffle<2, 0, 3, 1>(v);
}

static AI Sk4f nextCCW(const Sk4f& v) {
    return SkNx_shuffle<1, 3, 0, 2>(v);
}

// Fills Sk4f with 1f if edge bit is set, 0f otherwise. Edges are ordered LBTR to match CCW ordering
// of vertices in the quad.
static AI Sk4f compute_edge_mask(GrQuadAAFlags aaFlags) {
    return Sk4f((GrQuadAAFlags::kLeft & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kBottom & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kTop & aaFlags) ? 1.f : 0.f,
                (GrQuadAAFlags::kRight & aaFlags) ? 1.f : 0.f);
}

// Outputs normalized edge vectors in xdiff and ydiff, as well as the reciprocal of the original
// edge lengths in invLengths
static AI void compute_edge_vectors(const Sk4f& x, const Sk4f& y, const Sk4f& xnext,
                                    const Sk4f& ynext, Sk4f* xdiff, Sk4f* ydiff, Sk4f* invLengths) {
    *xdiff = xnext - x;
    *ydiff = ynext - y;
    *invLengths = fma(*xdiff, *xdiff, *ydiff * *ydiff).rsqrt();
    *xdiff *= *invLengths;
    *ydiff *= *invLengths;
}

static AI void outset_masked_vertices(float outset, const Sk4f& xdiff, const Sk4f& ydiff,
                                      const Sk4f& invLengths, const Sk4f& mask, Sk4f* x, Sk4f* y,
                                      Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    auto outsetMask = outset * mask;
    auto maskCW = nextCW(outsetMask);
    *x += maskCW * -xdiff + outsetMask * nextCW(xdiff);
    *y += maskCW * -ydiff + outsetMask * nextCW(ydiff);
    if (uvrCount > 0) {
        // We want to extend the texture coords by the same proportion as the positions.
        maskCW *= invLengths;
        outsetMask *= nextCW(invLengths);
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += maskCW * -udiff + outsetMask * nextCW(udiff);
        *v += maskCW * -vdiff + outsetMask * nextCW(vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += maskCW * -rdiff + outsetMask * nextCW(rdiff);
        }
    }
}

static AI void outset_vertices(float outset, const Sk4f& xdiff, const Sk4f& ydiff,
                               const Sk4f& invLengths, Sk4f* x, Sk4f* y, Sk4f* u, Sk4f* v, Sk4f* r,
                               int uvrCount) {
    *x += outset * (-xdiff + nextCW(xdiff));
    *y += outset * (-ydiff + nextCW(ydiff));
    if (uvrCount > 0) {
        Sk4f t = outset * invLengths;
        Sk4f udiff = nextCCW(*u) - *u;
        Sk4f vdiff = nextCCW(*v) - *v;
        *u += t * -udiff + nextCW(t) * nextCW(udiff);
        *v += t * -vdiff + nextCW(t) * nextCW(vdiff);
        if (uvrCount == 3) {
            Sk4f rdiff = nextCCW(*r) - *r;
            *r += t * -rdiff + nextCW(t) * nextCW(rdiff);
        }
    }
}

static AI float get_max_coverage(const Sk4f& lengths) {
    // Use the longest of opposite sides for width and height. This is to prevent the situation
    // where a quad under high-perspective has one edge that is sub-pixel and the other not. If
    // we used the minimum edge, the entire quad would have partial coverage over its pixels,
    // whereas using the maximum ensures full coverage in that situation, but still does partial
    // coverage when it is truly a subpixel rectangle.
    float width = SkMinScalar(lengths[0], lengths[3]);
    float height = SkMinScalar(lengths[1], lengths[2]);
    // Calculate approximate area of the quad, pinning dimensions to 1 in case the quad is larger
    // than a pixel. Sub-pixel quads that are rotated may in fact have a different true maximum
    // coverage than this calculation, but this will be close and is stable.
    return SkMinScalar(width, 1.f) * SkMinScalar(height, 1.f);
}

static AI float get_max_coverage(const Sk4f& x, const Sk4f& y) {
    Sk4f xdiff = nextCCW(x) - x;
    Sk4f ydiff = nextCCW(y) - y;
    Sk4f lengths = fma(xdiff, xdiff, ydiff * ydiff).sqrt();
    return get_max_coverage(lengths);
}

// Computes the vertices for the two nested quads used to create AA edges. The original single quad
// should be duplicated as input in x1 and x2, y1 and y2, and possibly u1|u2, v1|v2, [r1|r2]
// (controlled by uvrChannelCount).  While the values should be duplicated, they should be separate
// pointers. The outset quad is written in-place back to x1, y1, etc. and the inset inner quad is
// written to x2, y2, etc.
static float compute_nested_quad_vertices(GrQuadAAFlags aaFlags, Sk4f* x1, Sk4f* y1,
        Sk4f* u1, Sk4f* v1, Sk4f* r1, Sk4f* x2, Sk4f* y2, Sk4f* u2, Sk4f* v2, Sk4f* r2,
        int uvrCount) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    // Compute edge vectors for the quad.
    auto xnext = nextCCW(*x1);
    auto ynext = nextCCW(*y1);
    // xdiff and ydiff will comprise the normalized vectors pointing along each quad edge.
    Sk4f xdiff, ydiff, invLengths;
    compute_edge_vectors(*x1, *y1, xnext, ynext, &xdiff, &ydiff, &invLengths);
    float maxCoverage = get_max_coverage(invLengths.invert());

    if (aaFlags != GrQuadAAFlags::kAll) {
        Sk4f mask = compute_edge_mask(aaFlags);
        // outset by .5 pixel for the outer quad and then outset by -.5 pixel for the inner quad
        // (so insetting, but also clamping the amount of insetting to make sure subpixel quads
        // do not flip).
        outset_masked_vertices(0.5f, xdiff, ydiff, invLengths, mask, x1, y1, u1, v1, r1, uvrCount);
        outset_masked_vertices(-.5f * maxCoverage, xdiff, ydiff, invLengths, mask, x2, y2,
                               u2, v2, r2, uvrCount);
    } else {
        outset_vertices(0.5f, xdiff, ydiff, invLengths, x1, y1, u1, v1, r1, uvrCount);
        outset_vertices(-.5f * maxCoverage, xdiff, ydiff, invLengths, x2, y2, u2, v2, r2, uvrCount);
    }

    return maxCoverage;
}

// Generalizes compute_nested_quad_vertices to extrapolate local coords such that
// after perspective division of the device coordinate, the original local coordinate value is at
// the original un-outset device position. r is the local coordinate's w component. However, since
// the projected edges will be different for inner and outer quads, there isn't much reuse between
// the calculations, so it's easier to just have this operate on one quad a time.
static float compute_quad_persp_vertices(float outset, GrQuadAAFlags aaFlags, Sk4f* x, Sk4f* y,
        Sk4f* w, Sk4f* u, Sk4f* v, Sk4f* r, int uvrCount) {
    SkASSERT(uvrCount == 0 || uvrCount == 2 || uvrCount == 3);

    auto iw = (*w).invert();
    auto x2d = (*x) * iw;
    auto y2d = (*y) * iw;

    // Only compute the coverage limit when shrinking the quad
    float maxProjectedCoverage = outset < 0 ? get_max_coverage(x2d, y2d) : 1.f;
    outset *= maxProjectedCoverage;

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
        Sk4f s = SkNx_shuffle<0, 1, 0, 1>((len + outset) / len);
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

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<2, 3, 0, 1>(*u);
            Sk4f opV = SkNx_shuffle<2, 3, 0, 1>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
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

        Sk4f s = SkNx_shuffle<0, 0, 1, 1>((len + outset) / len);

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

        if (uvrCount > 0) {
            Sk4f opU = SkNx_shuffle<1, 0, 3, 2>(*u);
            Sk4f opV = SkNx_shuffle<1, 0, 3, 2>(*v);
            *u = opU + t * (*u - opU);
            *v = opV + t * (*v - opV);
            if (uvrCount == 3) {
                Sk4f opR = SkNx_shuffle<1, 0, 3, 2>(*r);
                *r = opR + t * (*r - opR);
            }
        }
    }

    return maxProjectedCoverage;
}

enum class CoverageMode {
    kNone,
    kWithPosition,
    kWithColor
};

static CoverageMode get_mode_for_spec(const GrQuadPerEdgeAA::VertexSpec& spec) {
    if (spec.usesCoverageAA()) {
        if (spec.compatibleWithAlphaAsCoverage() && spec.hasVertexColors()) {
            return CoverageMode::kWithColor;
        } else {
            return CoverageMode::kWithPosition;
        }
    } else {
        return CoverageMode::kNone;
    }
}

// Writes four vertices in triangle strip order, including the additional data for local
// coordinates, domain, color, and coverage as needed to satisfy the vertex spec.
static void write_quad(GrVertexWriter* vb, const GrQuadPerEdgeAA::VertexSpec& spec,
                       CoverageMode mode, float coverage,
                       SkPMColor4f color4f, bool wideColor,
                       const SkRect& domain,
                       const Sk4f& x, const Sk4f& y, const Sk4f& w,
                       const Sk4f& u, const Sk4f& v, const Sk4f& r) {
    static constexpr auto If = GrVertexWriter::If<float>;

    if (mode == CoverageMode::kWithColor) {
        // Multiply the color by the coverage up front
        SkASSERT(spec.hasVertexColors());
        color4f = color4f * coverage;
    }
    GrVertexColor color(color4f, wideColor);

    for (int i = 0; i < 4; ++i) {
        // save position, this is a float2 or float3 or float4 depending on the combination of
        // perspective and coverage mode.
        vb->write(x[i], y[i], If(spec.deviceQuadType() == GrQuadType::kPerspective, w[i]),
                  If(mode == CoverageMode::kWithPosition, coverage));

        // save color
        if (spec.hasVertexColors()) {
            vb->write(color);
        }

        // save local position
        if (spec.hasLocalCoords()) {
            vb->write(u[i], v[i], If(spec.localQuadType() == GrQuadType::kPerspective, r[i]));
        }

        // save the domain
        if (spec.hasDomain()) {
            vb->write(domain);
        }
    }
}

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static const int kVertsPerAAFillRect = 8;
static const int kIndicesPerAAFillRect = 30;

static sk_sp<const GrBuffer> get_index_buffer(GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

    // clang-format off
    static const uint16_t gFillAARectIdx[] = {
        0, 1, 2, 1, 3, 2,
        0, 4, 1, 4, 5, 1,
        0, 6, 4, 0, 2, 6,
        2, 3, 6, 3, 7, 6,
        1, 5, 3, 3, 5, 7,
    };
    // clang-format on

    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
    return resourceProvider->findOrCreatePatternedIndexBuffer(
            gFillAARectIdx, kIndicesPerAAFillRect, GrQuadPerEdgeAA::kNumAAQuadsInIndexBuffer,
            kVertsPerAAFillRect, gAAFillRectIndexBufferKey);
}

} // anonymous namespace

namespace GrQuadPerEdgeAA {

////////////////// Tessellate Implementation

void* Tessellate(void* vertices, const VertexSpec& spec, const GrPerspQuad& deviceQuad,
                 const SkPMColor4f& color4f, const GrPerspQuad& localQuad, const SkRect& domain,
                 GrQuadAAFlags aaFlags) {
    bool wideColor = GrQuadPerEdgeAA::ColorType::kHalf == spec.colorType();
    CoverageMode mode = get_mode_for_spec(spec);

    // Load position data into Sk4fs (always x, y, and load w to avoid branching down the road)
    Sk4f oX = deviceQuad.x4f();
    Sk4f oY = deviceQuad.y4f();
    Sk4f oW = deviceQuad.w4f(); // Guaranteed to be 1f if it's not perspective

    // Load local position data into Sk4fs (either none, just u,v or all three)
    Sk4f oU, oV, oR;
    if (spec.hasLocalCoords()) {
        oU = localQuad.x4f();
        oV = localQuad.y4f();
        oR = localQuad.w4f(); // Will be ignored if the local quad type isn't perspective
    }

    GrVertexWriter vb{vertices};
    if (spec.usesCoverageAA()) {
        SkASSERT(mode == CoverageMode::kWithPosition || mode == CoverageMode::kWithColor);

        // Must calculate two new quads, an outset and inset by .5 in projected device space, so
        // duplicate the original quad into new Sk4fs for the inset.
        Sk4f iX = oX, iY = oY, iW = oW;
        Sk4f iU = oU, iV = oV, iR = oR;

        float maxCoverage = 1.f;
        if (aaFlags != GrQuadAAFlags::kNone) {
            if (spec.deviceQuadType() == GrQuadType::kPerspective) {
                // Outset and inset the quads independently because perspective makes each shift
                // unique. Since iX copied pre-outset oX, this will compute the proper inset too.
                compute_quad_persp_vertices(
                        0.5f, aaFlags, &oX, &oY, &oW, &oU, &oV, &oW, spec.localDimensionality());
                // Save coverage limit when computing inset quad
                maxCoverage = compute_quad_persp_vertices(
                        -.5f, aaFlags, &iX, &iY, &iW, &iU, &iV, &iW, spec.localDimensionality());
            } else {
                // In the 2D case, insetting and outsetting can reuse the edge vectors, so the
                // nested quads are computed together
                maxCoverage = compute_nested_quad_vertices(aaFlags, &oX, &oY, &oU, &oV, &oR,
                                             &iX, &iY, &iU, &iV, &iR, spec.localDimensionality());
            }
            // FIXME handle rect fast-path (not the most trivial since idx0 is not always left edge)
        } // else don't adjust any positions, let the outer quad form degenerate triangles

        // Write two quads for inner and outer, inner will use the
        write_quad(&vb, spec, mode, maxCoverage, color4f, wideColor, domain,
                   iX, iY, iW, iU, iV, iR);
        write_quad(&vb, spec, mode, 0.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
    } else {
        // No outsetting needed, just write a single quad with full coverage
        SkASSERT(mode == CoverageMode::kNone);
        write_quad(&vb, spec, mode, 1.f, color4f, wideColor, domain, oX, oY, oW, oU, oV, oR);
    }

    return vb.fPtr;
}

bool ConfigureMeshIndices(GrMeshDrawOp::Target* target, GrMesh* mesh, const VertexSpec& spec,
                          int quadCount) {
    if (spec.usesCoverageAA()) {
        // AA quads use 8 vertices, basically nested rectangles
        sk_sp<const GrBuffer> ibuffer = get_index_buffer(target->resourceProvider());
        if (!ibuffer) {
            return false;
        }

        mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
        mesh->setIndexedPatterned(ibuffer.get(), kIndicesPerAAFillRect, kVertsPerAAFillRect,
                quadCount, kNumAAQuadsInIndexBuffer);
    } else {
        // Non-AA quads use 4 vertices, and regular triangle strip layout
        if (quadCount > 1) {
            sk_sp<const GrBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                return false;
            }

            mesh->setPrimitiveType(GrPrimitiveType::kTriangles);
            mesh->setIndexedPatterned(ibuffer.get(), 6, 4, quadCount,
                                      GrResourceProvider::QuadCountOfQuadBuffer());
        } else {
            mesh->setPrimitiveType(GrPrimitiveType::kTriangleStrip);
            mesh->setNonIndexedNonInstanced(4);
        }
    }

    return true;
}

////////////////// VertexSpec Implementation

int VertexSpec::deviceDimensionality() const {
    return this->deviceQuadType() == GrQuadType::kPerspective ? 3 : 2;
}

int VertexSpec::localDimensionality() const {
    return fHasLocalCoords ? (this->localQuadType() == GrQuadType::kPerspective ? 3 : 2) : 0;
}

////////////////// Geometry Processor Implementation

class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& spec) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(spec));
    }

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& vertexSpec, const GrShaderCaps& caps,
                                           GrTextureType textureType, GrPixelConfig textureConfig,
                                           const GrSamplerState& samplerState,
                                           uint32_t extraSamplerKey,
                                           sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                vertexSpec, caps, textureType, textureConfig, samplerState, extraSamplerKey,
                std::move(textureColorSpaceXform)));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // domain, texturing, device-dimensions are single bit flags
        uint32_t x = fDomain.isInitialized() ? 0 : 1;
        x |= fSampler.isInitialized() ? 0 : 2;
        x |= fNeedsPerspective ? 0 : 4;
        // local coords require 2 bits (3 choices), 00 for none, 01 for 2d, 10 for 3d
        if (fLocalCoord.isInitialized()) {
            x |= kFloat3_GrVertexAttribType == fLocalCoord.cpuType() ? 8 : 16;
        }
        // similar for colors, 00 for none, 01 for bytes, 10 for half-floats
        if (fColor.isInitialized()) {
            x |= kUByte4_norm_GrVertexAttribType == fColor.cpuType() ? 32 : 64;
        }
        // and coverage mode, 00 for none, 01 for withposition, 10 for withcolor
        if (fCoverageMode != CoverageMode::kNone) {
            x |= CoverageMode::kWithPosition == fCoverageMode ? 128 : 256;
        }

        b->add32(GrColorSpaceXform::XformKey(fTextureColorSpaceXform.get()));
        b->add32(x);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& gp = proc.cast<QuadPerEdgeAAGeometryProcessor>();
                if (gp.fLocalCoord.isInitialized()) {
                    this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                }
                fTextureColorSpaceXformHelper.setData(pdman, gp.fTextureColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                using Interpolation = GrGLSLVaryingHandler::Interpolation;

                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                fTextureColorSpaceXformHelper.emitCode(args.fUniformHandler,
                                                       gp.fTextureColorSpaceXform.get());

                args.fVaryingHandler->emitAttributes(gp);

                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    // Strip last channel from the vertex attribute to remove coverage and get the
                    // actual position
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("float3 position = %s.xyz;",
                                                       gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("float2 position = %s.xy;",
                                                       gp.fPosition.name());
                    }
                    gpArgs->fPositionVar = {"position",
                                            gp.fNeedsPerspective ? kFloat3_GrSLType
                                                                 : kFloat2_GrSLType,
                                            GrShaderVar::kNone_TypeModifier};
                } else {
                    // No coverage to eliminate
                    gpArgs->fPositionVar = gp.fPosition.asShaderVar();
                }

                // Handle local coordinates if they exist
                if (gp.fLocalCoord.isInitialized()) {
                    // NOTE: If the only usage of local coordinates is for the inline texture fetch
                    // before FPs, then there are no registered FPCoordTransforms and this ends up
                    // emitting nothing, so there isn't a duplication of local coordinates
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fLocalCoord.asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                // Solid color before any texturing gets modulated in
                if (gp.fColor.isInitialized()) {
                    // The color cannot be flat if the varying coverage has been modulated into it
                    args.fVaryingHandler->addPassThroughAttribute(gp.fColor, args.fOutputColor,
                            gp.fCoverageMode == CoverageMode::kWithColor ?
                            Interpolation::kInterpolated : Interpolation::kCanBeFlat);
                } else {
                    // Output color must be initialized to something
                    args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputColor);
                }

                // If there is a texture, must also handle texture coordinates and reading from
                // the texture in the fragment shader before continuing to fragment processors.
                if (gp.fSampler.isInitialized()) {
                    // Texture coordinates clamped by the domain on the fragment shader; if the GP
                    // has a texture, it's guaranteed to have local coordinates
                    args.fFragBuilder->codeAppend("float2 texCoord;");
                    if (gp.fLocalCoord.cpuType() == kFloat3_GrVertexAttribType) {
                        // Can't do a pass through since we need to perform perspective division
                        GrGLSLVarying v(gp.fLocalCoord.gpuType());
                        args.fVaryingHandler->addVarying(gp.fLocalCoord.name(), &v);
                        args.fVertBuilder->codeAppendf("%s = %s;",
                                                       v.vsOut(), gp.fLocalCoord.name());
                        args.fFragBuilder->codeAppendf("texCoord = %s.xy / %s.z;",
                                                       v.fsIn(), v.fsIn());
                    } else {
                        args.fVaryingHandler->addPassThroughAttribute(gp.fLocalCoord, "texCoord");
                    }

                    // Clamp the now 2D localCoordName variable by the domain if it is provided
                    if (gp.fDomain.isInitialized()) {
                        args.fFragBuilder->codeAppend("float4 domain;");
                        args.fVaryingHandler->addPassThroughAttribute(gp.fDomain, "domain",
                                                                      Interpolation::kCanBeFlat);
                        args.fFragBuilder->codeAppend(
                                "texCoord = clamp(texCoord, domain.xy, domain.zw);");
                    }

                    // Now modulate the starting output color by the texture lookup
                    args.fFragBuilder->codeAppendf("%s = ", args.fOutputColor);
                    args.fFragBuilder->appendTextureLookupAndModulate(
                        args.fOutputColor, args.fTexSamplers[0], "texCoord", kFloat2_GrSLType,
                        &fTextureColorSpaceXformHelper);
                    args.fFragBuilder->codeAppend(";");
                }

                // And lastly, output the coverage calculation code
                if (gp.fCoverageMode == CoverageMode::kWithPosition) {
                    GrGLSLVarying coverage(kFloat_GrSLType);
                    args.fVaryingHandler->addVarying("coverage", &coverage);
                    if (gp.fNeedsPerspective) {
                        args.fVertBuilder->codeAppendf("%s = %s.w;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    } else {
                        args.fVertBuilder->codeAppendf("%s = %s.z;",
                                                       coverage.vsOut(), gp.fPosition.name());
                    }

                    args.fFragBuilder->codeAppendf("%s = float4(%s);",
                                                   args.fOutputCoverage, coverage.fsIn());
                } else {
                    // Set coverage to 1, since it's either non-AA or the coverage was already
                    // folded into the output color
                    args.fFragBuilder->codeAppendf("%s = float4(1);", args.fOutputCoverage);
                }
            }
            GrGLSLColorSpaceXformHelper fTextureColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(nullptr) {
        SkASSERT(!spec.hasDomain());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(0);
    }

    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
                                   GrTextureType textureType, GrPixelConfig textureConfig,
                                   const GrSamplerState& samplerState,
                                   uint32_t extraSamplerKey,
                                   sk_sp<GrColorSpaceXform> textureColorSpaceXform)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fTextureColorSpaceXform(std::move(textureColorSpaceXform))
            , fSampler(textureType, textureConfig, samplerState, extraSamplerKey) {
        SkASSERT(spec.hasLocalCoords());
        this->initializeAttrs(spec);
        this->setTextureSamplerCnt(1);
    }

    void initializeAttrs(const VertexSpec& spec) {
        fNeedsPerspective = spec.deviceDimensionality() == 3;
        fCoverageMode = get_mode_for_spec(spec);

        if (fCoverageMode == CoverageMode::kWithPosition) {
            if (fNeedsPerspective) {
                fPosition = {"positionWithCoverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
            } else {
                fPosition = {"positionWithCoverage", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            }
        } else {
            if (fNeedsPerspective) {
                fPosition = {"position", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
            } else {
                fPosition = {"position", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
            }
        }

        int localDim = spec.localDimensionality();
        if (localDim == 3) {
            fLocalCoord = {"localCoord", kFloat3_GrVertexAttribType, kFloat3_GrSLType};
        } else if (localDim == 2) {
            fLocalCoord = {"localCoord", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
        } // else localDim == 0 and attribute remains uninitialized

        if (ColorType::kByte == spec.colorType()) {
            fColor = {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
        } else if (ColorType::kHalf == spec.colorType()) {
            fColor = {"color", kHalf4_GrVertexAttribType, kHalf4_GrSLType};
        }

        if (spec.hasDomain()) {
            fDomain = {"domain", kFloat4_GrVertexAttribType, kFloat4_GrSLType};
        }

        this->setVertexAttributes(&fPosition, 4);
    }

    const TextureSampler& onTextureSampler(int) const override { return fSampler; }

    Attribute fPosition; // May contain coverage as last channel
    Attribute fColor; // May have coverage modulated in if the FPs support it
    Attribute fLocalCoord;
    Attribute fDomain;

    // The positions attribute may have coverage built into it, so float3 is an ambiguous type
    // and may mean 2d with coverage, or 3d with no coverage
    bool fNeedsPerspective;
    CoverageMode fCoverageMode;

    // Color space will be null and fSampler.isInitialized() returns false when the GP is configured
    // to skip texturing.
    sk_sp<GrColorSpaceXform> fTextureColorSpaceXform;
    TextureSampler fSampler;

    typedef GrGeometryProcessor INHERITED;
};

sk_sp<GrGeometryProcessor> MakeProcessor(const VertexSpec& spec) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec);
}

sk_sp<GrGeometryProcessor> MakeTexturedProcessor(const VertexSpec& spec, const GrShaderCaps& caps,
        GrTextureType textureType, GrPixelConfig textureConfig,
        const GrSamplerState& samplerState, uint32_t extraSamplerKey,
        sk_sp<GrColorSpaceXform> textureColorSpaceXform) {
    return QuadPerEdgeAAGeometryProcessor::Make(spec, caps, textureType, textureConfig,
                                                samplerState, extraSamplerKey,
                                                std::move(textureColorSpaceXform));
}

} // namespace GrQuadPerEdgeAA
