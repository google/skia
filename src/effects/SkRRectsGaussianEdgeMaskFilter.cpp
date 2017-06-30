/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRRectsGaussianEdgeMaskFilter.h"
#include "SkReadBuffer.h"
#include "SkRRect.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

 /** \class SkRRectsGaussianEdgeMaskFilterImpl
  * This mask filter applies a gaussian edge to the intersection of two round rects.
  * The round rects must have the same radii at each corner and the x&y radii
  * must also be equal.
  */
class SkRRectsGaussianEdgeMaskFilterImpl : public SkMaskFilter {
public:
    SkRRectsGaussianEdgeMaskFilterImpl(const SkRRect& first, const SkRRect& second,
                                       SkScalar radius)
        : fFirst(first)
        , fSecond(second)
        , fRadius(radius) {
    }

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU
    bool asFragmentProcessor(GrFragmentProcessor**) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRRectsGaussianEdgeMaskFilterImpl)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkRRect  fFirst;
    SkRRect  fSecond;
    SkScalar fRadius;

    friend class SkRRectsGaussianEdgeMaskFilter; // for serialization registration system

    typedef SkMaskFilter INHERITED;
};

// x & y are in device space
static SkScalar compute_rrect_normalized_dist(const SkRRect& rr, const SkPoint& p, SkScalar rad) {
    SkASSERT(rr.getType() == SkRRect::kOval_Type || rr.getType() == SkRRect::kRect_Type ||
             rr.getType() == SkRRect::kSimple_Type);
    SkASSERT(rad > 0.0f);

    SkVector delta = { SkTAbs(p.fX - rr.rect().centerX()), SkTAbs(p.fY - rr.rect().centerY()) };

    SkScalar halfW = 0.5f * rr.rect().width();
    SkScalar halfH = 0.5f * rr.rect().height();
    SkScalar invRad = 1.0f/rad;

    const SkVector& radii = rr.getSimpleRadii();
    SkASSERT(SkScalarNearlyEqual(radii.fX, radii.fY));

    switch (rr.getType()) {
        case SkRRect::kOval_Type: {
            float scaledDist = delta.length() * invRad;
            return SkTPin(halfW * invRad - scaledDist, 0.0f, 1.0f);
        }
        case SkRRect::kRect_Type: {
            SkScalar xDist = (halfW - delta.fX) * invRad;
            SkScalar yDist = (halfH - delta.fY) * invRad;

            SkVector v = { 1.0f - SkTPin(xDist, 0.0f, 1.0f), 1.0f - SkTPin(yDist, 0.0f, 1.0f) };
            return SkTPin(1.0f - v.length(), 0.0f, 1.0f);
        }
        case SkRRect::kSimple_Type: {

            //----------------
            // ice-cream-cone fractional distance computation

            // When the blurRadius is larger than the corner radius we want to use it to
            // compute the pointy end of the ice cream cone. If it smaller we just want to use
            // the center of the corner's circle. When using the blurRadius the inset amount
            // can't exceed the halfwidths of the RRect.
            SkScalar insetDist = SkTMin(SkTMax(rad, radii.fX), SkTMin(halfW, halfH));

            // "maxValue" is a correction term for if the blurRadius is larger than the
            // size of the RRect. In that case we don't want to go all the way to black.
            SkScalar maxValue = insetDist * invRad;

            SkVector coneBottom = { halfW - insetDist, halfH - insetDist };
            SkVector ptInConeSpace = delta - coneBottom;

            SkVector cornerTop = { halfW - radii.fX - coneBottom.fX, halfH - coneBottom.fY };
            SkVector cornerRight = { halfW - coneBottom.fX, halfH - radii.fY - coneBottom.fY };

            SkScalar cross1 = ptInConeSpace.cross(cornerTop);
            SkScalar cross2 = cornerRight.cross(ptInConeSpace);
            bool inCone = cross1 > 0.0f && cross2 > 0.0f;

            if (!inCone) {
                SkScalar xDist = (halfW - delta.fX) * invRad;
                SkScalar yDist = (halfH - delta.fY) * invRad;

                return SkTPin(SkTMin(xDist, yDist), 0.0f, 1.0f); // perpendicular distance
            }

            SkVector cornerCenterInConeSpace = { insetDist - radii.fX, insetDist - radii.fY };

            SkVector connectingVec = ptInConeSpace - cornerCenterInConeSpace;
            float distToPtInConeSpace = SkPoint::Normalize(&ptInConeSpace);

            // "a" (i.e., dot(ptInConeSpace, ptInConeSpace) should always be 1.0f since
            // ptInConeSpace is now normalized
            SkScalar b = 2.0f * ptInConeSpace.dot(connectingVec);
            SkScalar c = connectingVec.dot(connectingVec) - radii.fX * radii.fY;

            // lop off negative values that are outside the cone
            SkScalar coneDist = SkTMax(0.0f, 0.5f * (-b + SkScalarSqrt(b*b - 4*c)));

            // make the coneDist a fraction of how far it is from the edge to the cone's base
            coneDist = (maxValue*coneDist) / (coneDist+distToPtInConeSpace);
            return SkTPin(coneDist, 0.0f, 1.0f);
        }
        default:
            return 0.0f;
    }
}

bool SkRRectsGaussianEdgeMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                                    const SkMatrix& matrix,
                                                    SkIPoint* margin) const {

    if (src.fFormat != SkMask::kA8_Format) {
        return false;
    }

    if (margin) {
        margin->set(0, 0);
    }

    dst->fBounds = src.fBounds;
    dst->fRowBytes = dst->fBounds.width();
    dst->fFormat = SkMask::kA8_Format;
    dst->fImage = nullptr;

    if (src.fImage) {
        size_t dstSize = dst->computeImageSize();
        if (0 == dstSize) {
            return false;   // too big to allocate, abort
        }

        const uint8_t* srcPixels = src.fImage;
        uint8_t* dstPixels = dst->fImage = SkMask::AllocImage(dstSize);

        SkPoint basePt = { SkIntToScalar(src.fBounds.fLeft), SkIntToScalar(src.fBounds.fTop) };

        for (int y = 0; y < dst->fBounds.height(); ++y) {
            const uint8_t* srcRow = srcPixels + y * dst->fRowBytes;
            uint8_t* dstRow = dstPixels + y*dst->fRowBytes;

            for (int x = 0; x < dst->fBounds.width(); ++x) {
                SkPoint curPt = { basePt.fX + x, basePt.fY + y };

                SkVector vec;
                vec.fX = 1.0f - compute_rrect_normalized_dist(fFirst, curPt, fRadius);
                vec.fY = 1.0f - compute_rrect_normalized_dist(fSecond, curPt, fRadius);

                SkScalar factor = SkTPin(vec.length(), 0.0f, 1.0f);
                factor = exp(-factor * factor * 4.0f) - 0.018f;
                SkASSERT(factor >= 0.0f && factor <= 1.0f);

                dstRow[x] = (uint8_t) (factor * srcRow[x]);
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"

class RRectsGaussianEdgeFP : public GrFragmentProcessor {
public:
    enum Mode {
        kCircle_Mode,
        kRect_Mode,
        kSimpleCircular_Mode,
    };

    static sk_sp<GrFragmentProcessor> Make(const SkRRect& first, const SkRRect& second,
                                           SkScalar radius) {
        return sk_sp<GrFragmentProcessor>(new RRectsGaussianEdgeFP(first, second, radius));
    }

    class GLSLRRectsGaussianEdgeFP : public GrGLSLFragmentProcessor {
    public:
        GLSLRRectsGaussianEdgeFP() { }

        // This method emits code so that, for each shape, the distance from the edge is returned
        // in 'outputName' clamped to 0..1 with positive distance being towards the center of the
        // shape. The distance will have been normalized by the radius.
        void emitModeCode(Mode mode,
                          GrGLSLFPFragmentBuilder* fragBuilder,
                          const char* posName,
                          const char* sizesName,
                          const char* radiiName,
                          const char* radName,
                          const char* outputName,
                          const char  indices[2]) { // how to access the params for the 2 rrects

            // Positive distance is towards the center of the circle.
            // Map all the cases to the lower right quadrant.
            fragBuilder->codeAppendf("vec2 delta = abs(sk_FragCoord.xy - %s.%s);",
                                     posName, indices);

            switch (mode) {
                case kCircle_Mode:
                    // When a shadow circle gets large we can have some precision issues if
                    // we do "length(delta)/radius". The scaleDist temporary cuts the
                    // delta vector down a bit before invoking length.
                    fragBuilder->codeAppendf("float scaledDist = length(delta/%s);", radName);
                    fragBuilder->codeAppendf("%s = clamp((%s.%c/%s - scaledDist), 0.0, 1.0);",
                                             outputName, sizesName, indices[0], radName);
                    break;
                case kRect_Mode:
                    fragBuilder->codeAppendf(
                        "vec2 rectDist = vec2(1.0 - clamp((%s.%c - delta.x)/%s, 0.0, 1.0),"
                                             "1.0 - clamp((%s.%c - delta.y)/%s, 0.0, 1.0));",
                        sizesName, indices[0], radName,
                        sizesName, indices[1], radName);
                    fragBuilder->codeAppendf("%s = clamp(1.0 - length(rectDist), 0.0, 1.0);",
                                             outputName);
                    break;
                case kSimpleCircular_Mode:
                    // For the circular round rect we combine 2 distances:
                    //    the fractional position from the corner inset point to the corner's circle
                    //    the minimum perpendicular distance to the bounding rectangle
                    // The first distance is used when the pixel is inside the ice-cream-cone-shaped
                    // portion of a corner. The second is used everywhere else.
                    // This is intended to approximate the interpolation pattern if we had
                    // tessellated this geometry into a RRect outside and a rect inside.

                    //----------------
                    // rect distance computation
                    fragBuilder->codeAppendf("float xDist = (%s.%c - delta.x) / %s;",
                                             sizesName, indices[0], radName);
                    fragBuilder->codeAppendf("float yDist = (%s.%c - delta.y) / %s;",
                                             sizesName, indices[1], radName);
                    fragBuilder->codeAppend("float rectDist = clamp(min(xDist, yDist), 0.0, 1.0);");

                    //----------------
                    // ice-cream-cone fractional distance computation

                    // When the blurRadius is larger than the corner radius we want to use it to
                    // compute the pointy end of the ice cream cone. If it smaller we just want to
                    // use the center of the corner's circle. When using the blurRadius the inset
                    // amount can't exceed the halfwidths of the RRect.
                    fragBuilder->codeAppendf("float insetDist = min(max(%s, %s.%c),"
                                                                   "min(%s.%c, %s.%c));",
                                             radName, radiiName, indices[0],
                                             sizesName, indices[0], sizesName, indices[1]);
                    // "maxValue" is a correction term for if the blurRadius is larger than the
                    // size of the RRect. In that case we don't want to go all the way to black.
                    fragBuilder->codeAppendf("float maxValue = insetDist/%s;", radName);

                    fragBuilder->codeAppendf("vec2 coneBottom = vec2(%s.%c - insetDist,"
                                                                    "%s.%c - insetDist);",
                                             sizesName, indices[0], sizesName, indices[1]);

                    fragBuilder->codeAppendf("vec2 cornerTop = vec2(%s.%c - %s.%c, %s.%c) -"
                                                                        "coneBottom;",
                                             sizesName, indices[0], radiiName, indices[0],
                                             sizesName, indices[1]);
                    fragBuilder->codeAppendf("vec2 cornerRight = vec2(%s.%c, %s.%c - %s.%c) -"
                                                                        "coneBottom;",
                                             sizesName, indices[0],
                                             sizesName, indices[1], radiiName, indices[1]);

                    fragBuilder->codeAppend("vec2 ptInConeSpace = delta - coneBottom;");
                    fragBuilder->codeAppend("float distToPtInConeSpace = length(ptInConeSpace);");

                    fragBuilder->codeAppend("float cross1 =  ptInConeSpace.x * cornerTop.y -"
                                                            "ptInConeSpace.y * cornerTop.x;");
                    fragBuilder->codeAppend("float cross2 = -ptInConeSpace.x * cornerRight.y + "
                                                            "ptInConeSpace.y * cornerRight.x;");

                    fragBuilder->codeAppend("float inCone = step(0.0, cross1) *"
                                                           "step(0.0, cross2);");

                    fragBuilder->codeAppendf("vec2 cornerCenterInConeSpace = vec2(insetDist -"
                                                                                 "%s.%c);",
                                             radiiName, indices[0]);

                    fragBuilder->codeAppend("vec2 connectingVec = ptInConeSpace -"
                                                                        "cornerCenterInConeSpace;");
                    fragBuilder->codeAppend("ptInConeSpace = normalize(ptInConeSpace);");

                    // "a" (i.e., dot(ptInConeSpace, ptInConeSpace) should always be 1.0f since
                    // ptInConeSpace is now normalized
                    fragBuilder->codeAppend("float b = 2.0 * dot(ptInConeSpace, connectingVec);");
                    fragBuilder->codeAppendf("float c = dot(connectingVec, connectingVec) - "
                                                                                   "%s.%c * %s.%c;",
                                             radiiName, indices[0], radiiName, indices[0]);

                    fragBuilder->codeAppend("float fourAC = 4*c;");
                    // This max prevents sqrt(-1) when outside the cone
                    fragBuilder->codeAppend("float bSq = max(b*b, fourAC);");

                    // lop off negative values that are outside the cone
                    fragBuilder->codeAppend("float coneDist = "
                                                    "max(0.0, 0.5 * (-b + sqrt(bSq - fourAC)));");
                    // make the coneDist a fraction of how far it is from the edge to the
                    // cone's base
                    fragBuilder->codeAppend("coneDist = (maxValue*coneDist) /"
                                                                "(coneDist+distToPtInConeSpace);");
                    fragBuilder->codeAppend("coneDist = clamp(coneDist, 0.0, 1.0);");

                    //----------------
                    fragBuilder->codeAppendf("%s = mix(rectDist, coneDist, inCone);", outputName);
                    break;
                }
        }

        void emitCode(EmitArgs& args) override {
            const RRectsGaussianEdgeFP& fp = args.fFp.cast<RRectsGaussianEdgeFP>();
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            const char* positionsUniName = nullptr;
            fPositionsUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                       "Positions", &positionsUniName);
            const char* sizesUniName = nullptr;
            fSizesUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                   kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                   "Sizes", &sizesUniName);
            const char* radiiUniName = nullptr;
            if (fp.fFirstMode == kSimpleCircular_Mode || fp.fSecondMode == kSimpleCircular_Mode) {
                fRadiiUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                       "Radii", &radiiUniName);
            }
            const char* radUniName = nullptr;
            fRadiusUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                    kFloat_GrSLType, kDefault_GrSLPrecision,
                                                    "Radius", &radUniName);

            fragBuilder->codeAppend("float firstDist;");
            fragBuilder->codeAppend("{");
            this->emitModeCode(fp.firstMode(), fragBuilder,
                               positionsUniName, sizesUniName, radiiUniName,
                               radUniName, "firstDist", "xy");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppend("float secondDist;");
            fragBuilder->codeAppend("{");
            this->emitModeCode(fp.secondMode(), fragBuilder,
                               positionsUniName, sizesUniName, radiiUniName,
                               radUniName, "secondDist", "zw");
            fragBuilder->codeAppend("}");

            fragBuilder->codeAppend("vec2 distVec = vec2(1.0 - firstDist, 1.0 - secondDist);");

            // Finally use the distance to apply the Gaussian edge
            fragBuilder->codeAppend("float factor = clamp(length(distVec), 0.0, 1.0);");
            fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
            fragBuilder->codeAppendf("%s = factor*%s;",
                                     args.fOutputColor, args.fInputColor);
        }

        static void GenKey(const GrProcessor& proc, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
            const RRectsGaussianEdgeFP& fp = proc.cast<RRectsGaussianEdgeFP>();

            b->add32(fp.firstMode() | (fp.secondMode() << 4));
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& proc) override {
            const RRectsGaussianEdgeFP& edgeFP = proc.cast<RRectsGaussianEdgeFP>();

            const SkRRect& first = edgeFP.first();
            const SkRRect& second = edgeFP.second();

            pdman.set4f(fPositionsUni,
                        first.getBounds().centerX(),
                        first.getBounds().centerY(),
                        second.getBounds().centerX(),
                        second.getBounds().centerY());

            pdman.set4f(fSizesUni, 
                        0.5f * first.rect().width(),
                        0.5f * first.rect().height(),
                        0.5f * second.rect().width(),
                        0.5f * second.rect().height());

            if (edgeFP.firstMode() == kSimpleCircular_Mode ||
                edgeFP.secondMode() == kSimpleCircular_Mode) {
                // This is a bit of overkill since fX should equal fY for both round rects but it
                // makes the shader code simpler.
                pdman.set4f(fRadiiUni, 
                            first.getSimpleRadii().fX,  first.getSimpleRadii().fY,
                            second.getSimpleRadii().fX, second.getSimpleRadii().fY);
            }

            pdman.set1f(fRadiusUni, edgeFP.radius());
        }

    private:
        // The centers of the two round rects (x1, y1, x2, y2)
        GrGLSLProgramDataManager::UniformHandle fPositionsUni;

        // The half widths and half heights of the two round rects (w1/2, h1/2, w2/2, h2/2)
        // For circles we still upload both width & height to simplify things
        GrGLSLProgramDataManager::UniformHandle fSizesUni;

        // The corner radii of the two round rects (rx1, ry1, rx2, ry2)
        // We upload both the x&y radii (although they are currently always the same) to make
        // the indexing in the shader code simpler. In some future world we could also support
        // non-circular corner round rects & ellipses.
        GrGLSLProgramDataManager::UniformHandle fRadiiUni;

        // The radius parameters (radius)
        GrGLSLProgramDataManager::UniformHandle fRadiusUni;

        typedef GrGLSLFragmentProcessor INHERITED;
    };

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLRRectsGaussianEdgeFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "RRectsGaussianEdgeFP"; }

    const SkRRect& first() const { return fFirst; }
    Mode firstMode() const { return fFirstMode; }
    const SkRRect& second() const { return fSecond; }
    Mode secondMode() const { return fSecondMode; }
    SkScalar radius() const { return fRadius; }

private:
    RRectsGaussianEdgeFP(const SkRRect& first, const SkRRect& second, SkScalar radius)
            : INHERITED(kCompatibleWithCoverageAsAlpha_OptimizationFlag)
            , fFirst(first)
            , fSecond(second)
            , fRadius(radius) {
        this->initClassID<RRectsGaussianEdgeFP>();

        fFirstMode = ComputeMode(fFirst);
        fSecondMode = ComputeMode(fSecond);
    }

    static Mode ComputeMode(const SkRRect& rr) {
        if (rr.isCircle()) {
            return kCircle_Mode;
        } else if (rr.isRect()) {
            return kRect_Mode;
        } else {
            SkASSERT(rr.isSimpleCircular());
            return kSimpleCircular_Mode;
        }
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLRRectsGaussianEdgeFP;
    }

    bool onIsEqual(const GrFragmentProcessor& proc) const override {
        const RRectsGaussianEdgeFP& edgeFP = proc.cast<RRectsGaussianEdgeFP>();
        return fFirst  == edgeFP.fFirst &&
               fSecond == edgeFP.fSecond && 
               fRadius == edgeFP.fRadius;
    }

    SkRRect  fFirst;
    Mode     fFirstMode;
    SkRRect  fSecond;
    Mode     fSecondMode;
    SkScalar fRadius;

    typedef GrFragmentProcessor INHERITED;
};

////////////////////////////////////////////////////////////////////////////
bool SkRRectsGaussianEdgeMaskFilterImpl::asFragmentProcessor(GrFragmentProcessor** fp) const {
    if (fp) {
        *fp = RRectsGaussianEdgeFP::Make(fFirst, fSecond, fRadius).release();
    }

    return true;
}

#endif

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkRRectsGaussianEdgeMaskFilterImpl::toString(SkString* str) const {
    str->appendf("RRectsGaussianEdgeMaskFilter: ()");
}
#endif

sk_sp<SkFlattenable> SkRRectsGaussianEdgeMaskFilterImpl::CreateProc(SkReadBuffer& buf) {
    SkRect rect1, rect2;

    buf.readRect(&rect1);
    SkScalar xRad1 = buf.readScalar();
    SkScalar yRad1 = buf.readScalar();

    buf.readRect(&rect2);
    SkScalar xRad2 = buf.readScalar();
    SkScalar yRad2 = buf.readScalar();

    SkScalar radius = buf.readScalar();

    return sk_make_sp<SkRRectsGaussianEdgeMaskFilterImpl>(SkRRect::MakeRectXY(rect1, xRad1, yRad1),
                                                          SkRRect::MakeRectXY(rect2, xRad2, yRad2),
                                                          radius);
}

void SkRRectsGaussianEdgeMaskFilterImpl::flatten(SkWriteBuffer& buf) const {
    INHERITED::flatten(buf);

    SkASSERT(fFirst.isRect() || fFirst.isCircle() || fFirst.isSimpleCircular());
    buf.writeRect(fFirst.rect());
    const SkVector& radii1 = fFirst.getSimpleRadii();
    buf.writeScalar(radii1.fX);
    buf.writeScalar(radii1.fY);

    SkASSERT(fSecond.isRect() || fSecond.isCircle() || fSecond.isSimpleCircular());
    buf.writeRect(fSecond.rect());
    const SkVector& radii2 = fSecond.getSimpleRadii();
    buf.writeScalar(radii2.fX);
    buf.writeScalar(radii2.fY);

    buf.writeScalar(fRadius);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkMaskFilter> SkRRectsGaussianEdgeMaskFilter::Make(const SkRRect& first,
                                                         const SkRRect& second,
                                                         SkScalar radius) {
    if ((!first.isRect()  && !first.isCircle()  && !first.isSimpleCircular()) || 
        (!second.isRect() && !second.isCircle() && !second.isSimpleCircular())) {
        // we only deal with the shapes where the x & y radii are equal 
        // and the same for all four corners
        return nullptr;
    }

    return sk_make_sp<SkRRectsGaussianEdgeMaskFilterImpl>(first, second, radius);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkRRectsGaussianEdgeMaskFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRRectsGaussianEdgeMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
