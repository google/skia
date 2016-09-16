/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGaussianEdgeShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

 /** \class SkGaussianEdgeShaderImpl
 This subclass of shader applies a Gaussian to shadow edge

 If largerBlur is false:
 The radius of the Gaussian blur is specified by the g value of the color, in 6.2 fixed point.
 For spot shadows, we increase the stroke width to set the shadow against the shape. This pad
 is specified by b, also in 6.2 fixed point. The r value represents the max final alpha.
 The incoming alpha should be 1.

 If largerBlur is true:
 The radius of the Gaussian blur is specified by the r & g values of the color in 14.2 fixed point.
 For spot shadows, we increase the stroke width to set the shadow against the shape. This pad
 is specified by b, also in 6.2 fixed point. The a value represents the max final alpha.

 LargerBlur will be removed once Android is migrated to the updated shader.
 */
class SkGaussianEdgeShaderImpl : public SkShader {
public:
    SkGaussianEdgeShaderImpl() {}

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGaussianEdgeShaderImpl)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend class SkGaussianEdgeShader;

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

class GaussianEdgeFP : public GrFragmentProcessor {
public:
    GaussianEdgeFP() {
        this->initClassID<GaussianEdgeFP>();

        // enable output of distance information for shape
        fUsesDistanceVectorField = true;
    }

    class GLSLGaussianEdgeFP : public GrGLSLFragmentProcessor {
    public:
        GLSLGaussianEdgeFP() {}

        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

            if (!args.fGpImplementsDistanceVector) {
                fragBuilder->codeAppendf("// GP does not implement fsDistanceVector - "
                                         " returning grey in GLSLGaussianEdgeFP\n");
                fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
                fragBuilder->codeAppendf("%s = vec4(0.0, 0.0, 0.0, color.r);", args.fOutputColor);
            } else {
                fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);
                fragBuilder->codeAppend("float radius = color.r*256.0*64.0 + color.g*64.0;");
                fragBuilder->codeAppend("float pad = color.b*64.0;");

                fragBuilder->codeAppendf("float factor = 1.0 - clamp((%s.z - pad)/radius, 0.0, 1.0);",
                                         fragBuilder->distanceVectorName());
                fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
                fragBuilder->codeAppendf("%s = factor*vec4(0.0, 0.0, 0.0, color.a);",
                                         args.fOutputColor);
            }
        }

        static void GenKey(const GrProcessor& proc, const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            // only one shader generated currently
            b->add32(0x0);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {}

        bool fLargerBlur;
    };

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLGaussianEdgeFP::GenKey(*this, caps, b);
    }

    const char* name() const override { return "GaussianEdgeFP"; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->mulByUnknownFourComponents();
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLGaussianEdgeFP();
    }

    bool onIsEqual(const GrFragmentProcessor& proc) const override { return true; }
};

////////////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkGaussianEdgeShaderImpl::asFragmentProcessor(const AsFPArgs&) const {
    return sk_make_sp<GaussianEdgeFP>();
}

#endif

////////////////////////////////////////////////////////////////////////////

bool SkGaussianEdgeShaderImpl::isOpaque() const {
    return false;
}

////////////////////////////////////////////////////////////////////////////

#ifndef SK_IGNORE_TO_STRING
void SkGaussianEdgeShaderImpl::toString(SkString* str) const {
    str->appendf("GaussianEdgeShader: ()");
}
#endif

sk_sp<SkFlattenable> SkGaussianEdgeShaderImpl::CreateProc(SkReadBuffer& buf) {
    return sk_make_sp<SkGaussianEdgeShaderImpl>();
}

void SkGaussianEdgeShaderImpl::flatten(SkWriteBuffer& buf) const {
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkGaussianEdgeShader::Make() {
    return sk_make_sp<SkGaussianEdgeShaderImpl>();
}

///////////////////////////////////////////////////////////////////////////////

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGaussianEdgeShader)
SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkGaussianEdgeShaderImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////
