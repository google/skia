/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRRectsGaussianEdgeShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

 /** \class SkRRectsGaussianEdgeShaderImpl
  * This shader applies a gaussian edge to the intersection of two round rects.
  * The round rects must have the same radii at each corner and the x&y radii
  * must also be equal.
  */
class SkRRectsGaussianEdgeShaderImpl : public SkShader {
public:
    SkRRectsGaussianEdgeShaderImpl(const SkRRect& first, const SkRRect& second, SkScalar radius)
        : fFirst(first)
        , fSecond(second)
        , fRadius(radius) {
    }

    bool isOpaque() const override { return false; }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class GaussianEdgeShaderContext : public SkShader::Context {
    public:
        GaussianEdgeShaderContext(const SkRRectsGaussianEdgeShaderImpl&, const ContextRec&);

        ~GaussianEdgeShaderContext() override { }

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

        uint32_t getFlags() const override { return 0; }

    private:
        SkColor                   fPaintColor;

        typedef SkShader::Context INHERITED;
    };

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRRectsGaussianEdgeShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

private:
    SkRRect  fFirst;
    SkRRect  fSecond;
    SkScalar fRadius;

    friend class SkRRectsGaussianEdgeShader; // for serialization registration system

    typedef SkShader INHERITED;
};

////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"
#include "SkGrPriv.h"

class RRectsGaussianEdgeFP : public GrFragmentProcessor {
public:
    enum Mode {
        kCircle_Mode,
        kRect_Mode,
        kSimpleCircular_Mode,
    };

    RRectsGaussianEdgeFP(const SkRRect& first, const SkRRect& second, SkScalar radius)
        : fFirst(first)
        , fSecond(second)
        , fRadius(radius) {
        this->initClassID<RRectsGaussianEdgeFP>();
        this->setWillReadFragmentPosition();

        fFirstMode = ComputeMode(fFirst);
        fSecondMode = ComputeMode(fSecond);
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

            // positive distance is towards the center of the circle
            fragBuilder->codeAppendf("vec2 delta = %s.xy - %s.%s;",
                                     fragBuilder->fragmentPosition(), posName, indices);

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
                    "vec2 rectDist = vec2(1.0 - clamp((%s.%c - abs(delta.x))/%s, 0.0, 1.0),"
                                         "1.0 - clamp((%s.%c - abs(delta.y))/%s, 0.0, 1.0));",
                    sizesName, indices[0], radName,
                    sizesName, indices[1], radName);
                fragBuilder->codeAppendf("%s = clamp(1.0 - length(rectDist), 0.0, 1.0);",
                                         outputName);
                break;
            case kSimpleCircular_Mode:
                // For the circular round rect we first compute the distance
                // to the rect. Then we compute a multiplier that is 1 if the
                // point is in one of the circular corners. We then compute the
                // distance from the corner and then use the multiplier to mask
                // between the two distances.
                fragBuilder->codeAppendf("float xDist = clamp((%s.%c - abs(delta.x))/%s,"
                                                             "0.0, 1.0);",
                                         sizesName, indices[0], radName);
                fragBuilder->codeAppendf("float yDist = clamp((%s.%c - abs(delta.y))/%s,"
                                                             "0.0, 1.0);",
                                         sizesName, indices[1], radName);
                fragBuilder->codeAppend("float rectDist = min(xDist, yDist);");

                fragBuilder->codeAppendf("vec2 cornerCenter = %s.%s - %s.%s;",
                                         sizesName, indices, radiiName, indices);
                fragBuilder->codeAppend("delta = vec2(abs(delta.x) - cornerCenter.x,"
                                                     "abs(delta.y) - cornerCenter.y);");
                fragBuilder->codeAppendf("xDist = %s.%c - abs(delta.x);", radiiName, indices[0]);
                fragBuilder->codeAppendf("yDist = %s.%c - abs(delta.y);", radiiName, indices[1]);
                fragBuilder->codeAppend("float cornerDist = min(xDist, yDist);");
                fragBuilder->codeAppend("float multiplier = step(0.0, cornerDist);");

                fragBuilder->codeAppendf("delta += %s.%s;", radiiName, indices);

                fragBuilder->codeAppendf("cornerDist = clamp((2.0 * %s.%c - length(delta))/%s,"
                                                             "0.0, 1.0);",
                                         radiiName, indices[0], radName);

                fragBuilder->codeAppendf("%s = (multiplier * cornerDist) +"
                                              "((1.0-multiplier) * rectDist);",
                                         outputName);
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

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const RRectsGaussianEdgeFP& fp = proc.cast<RRectsGaussianEdgeFP>();

            b->add32(fp.firstMode() | (fp.secondMode() << 4));
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
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
                             0.5f * first.getSimpleRadii().fX,
                             0.5f * first.getSimpleRadii().fY,
                             0.5f * second.getSimpleRadii().fX,
                             0.5f * second.getSimpleRadii().fY);
            }

            pdman.set1f(fRadiusUni, edgeFP.radius());
        }

    private:
        // The centers of the two round rects (x1, y1, x2, y2)
        GrGLSLProgramDataManager::UniformHandle fPositionsUni;

        // The half widths and half heights of the two round rects (w1/2, h1/2, w2/2, h2/2)
        // For circles we still upload both width & height to simplify things
        GrGLSLProgramDataManager::UniformHandle fSizesUni;

        // The half corner radii of the two round rects (rx1/2, ry1/2, rx2/2, ry2/2)
        // We upload both the x&y radii (although they are currently always the same) to make
        // the indexing in the shader code simpler. In some future world we could also support
        // non-circular corner round rects & ellipses.
        GrGLSLProgramDataManager::UniformHandle fRadiiUni;

        // The radius parameters (radius)
        GrGLSLProgramDataManager::UniformHandle fRadiusUni;

        typedef GrGLSLFragmentProcessor INHERITED;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLRRectsGaussianEdgeFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "RRectsGaussianEdgeFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

    const SkRRect& first() const { return fFirst; }
    Mode firstMode() const { return fFirstMode; }
    const SkRRect& second() const { return fSecond; }
    Mode secondMode() const { return fSecondMode; }
    SkScalar radius() const { return fRadius; }

private:
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

sk_sp<GrFragmentProcessor> SkRRectsGaussianEdgeShaderImpl::asFragmentProcessor(
                                                                    const AsFPArgs& args) const {
    return sk_make_sp<RRectsGaussianEdgeFP>(fFirst, fSecond, fRadius);
}

#endif

////////////////////////////////////////////////////////////////////////////

SkRRectsGaussianEdgeShaderImpl::GaussianEdgeShaderContext::GaussianEdgeShaderContext(
                                                    const SkRRectsGaussianEdgeShaderImpl& shader,
                                                    const ContextRec& rec)
    : INHERITED(shader, rec) {

    fPaintColor = rec.fPaint->getColor();
}

void SkRRectsGaussianEdgeShaderImpl::GaussianEdgeShaderContext::shadeSpan(int x, int y,
                                                                          SkPMColor result[],
                                                                          int count) {
    // TODO: implement
    for (int i = 0; i < count; ++i) {
        result[i] = fPaintColor;
    }
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkRRectsGaussianEdgeShaderImpl::toString(SkString* str) const {
    str->appendf("RRectsGaussianEdgeShader: ()");
}
#endif

sk_sp<SkFlattenable> SkRRectsGaussianEdgeShaderImpl::CreateProc(SkReadBuffer& buf) {
    // Discarding SkShader flattenable params
    bool hasLocalMatrix = buf.readBool();
    SkAssertResult(!hasLocalMatrix);

    SkRect rect1, rect2;

    buf.readRect(&rect1);
    SkScalar xRad1 = buf.readScalar();
    SkScalar yRad1 = buf.readScalar();

    buf.readRect(&rect2);
    SkScalar xRad2 = buf.readScalar();
    SkScalar yRad2 = buf.readScalar();

    SkScalar radius = buf.readScalar();

    return sk_make_sp<SkRRectsGaussianEdgeShaderImpl>(SkRRect::MakeRectXY(rect1, xRad1, yRad1),
                                                      SkRRect::MakeRectXY(rect2, xRad2, yRad2),
                                                      radius);
}

void SkRRectsGaussianEdgeShaderImpl::flatten(SkWriteBuffer& buf) const {
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

size_t SkRRectsGaussianEdgeShaderImpl::onContextSize(const ContextRec& rec) const {
    return sizeof(GaussianEdgeShaderContext);
}

SkShader::Context* SkRRectsGaussianEdgeShaderImpl::onCreateContext(const ContextRec& rec,
                                                                   void* storage) const {
    return new (storage) GaussianEdgeShaderContext(*this, rec);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkRRectsGaussianEdgeShader::Make(const SkRRect& first,
                                                 const SkRRect& second,
                                                 SkScalar radius, SkScalar unused) {
    if ((!first.isRect()  && !first.isCircle()  && !first.isSimpleCircular()) || 
        (!second.isRect() && !second.isCircle() && !second.isSimpleCircular())) {
        // we only deal with the shapes where the x & y radii are equal 
        // and the same for all four corners
        return nullptr;
    }

    return sk_make_sp<SkRRectsGaussianEdgeShaderImpl>(first, second, radius);
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkRRectsGaussianEdgeShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRRectsGaussianEdgeShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
