/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLumaColorFilter.h"

#include "SkColorPriv.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrInvariantOutput.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#endif

void SkLumaColorFilter::filterSpan(const SkPMColor src[], int count,
                                   SkPMColor dst[]) const {
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];

        /*
         * While LuminanceToAlpha is defined to operate on un-premultiplied
         * inputs, due to the final alpha scaling it can be computed based on
         * premultipled components:
         *
         *   LumA = (k1 * r / a + k2 * g / a + k3 * b / a) * a
         *   LumA = (k1 * r + k2 * g + k3 * b)
         */
        unsigned luma = SkComputeLuminance(SkGetPackedR32(c),
                                           SkGetPackedG32(c),
                                           SkGetPackedB32(c));
        dst[i] = SkPackARGB32(luma, 0, 0, 0);
    }
}

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    return sk_sp<SkColorFilter>(new SkLumaColorFilter);
}

SkLumaColorFilter::SkLumaColorFilter() : INHERITED() {}

sk_sp<SkFlattenable> SkLumaColorFilter::CreateProc(SkReadBuffer&) {
    return Make();
}

void SkLumaColorFilter::flatten(SkWriteBuffer&) const {}

#ifndef SK_IGNORE_TO_STRING
void SkLumaColorFilter::toString(SkString* str) const {
    str->append("SkLumaColorFilter ");
}
#endif

#if SK_SUPPORT_GPU
class LumaColorFilterEffect : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make() {
        return sk_sp<GrFragmentProcessor>(new LumaColorFilterEffect);
    }

    const char* name() const override { return "Luminance-to-Alpha"; }

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

        void emitCode(EmitArgs& args) override {
            if (nullptr == args.fInputColor) {
                args.fInputColor = "vec4(1)";
            }

            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            fragBuilder->codeAppendf("\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb);\n",
                                     SK_ITU_BT709_LUM_COEFF_R,
                                     SK_ITU_BT709_LUM_COEFF_G,
                                     SK_ITU_BT709_LUM_COEFF_B,
                                     args.fInputColor);
            fragBuilder->codeAppendf("\t%s = vec4(0, 0, 0, luma);\n",
                                     args.fOutputColor);

        }

    private:
        typedef GrGLSLFragmentProcessor INHERITED;
    };

private:
    LumaColorFilterEffect() {
        this->initClassID<LumaColorFilterEffect>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLProcessor;
    }

    virtual void onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // The output is always black. The alpha value for the color passed in is arbitrary.
        inout->setToOther(kRGB_GrColorComponentFlags, GrColorPackRGBA(0, 0, 0, 0),
                          GrInvariantOutput::kWill_ReadInput);
    }
};

sk_sp<GrFragmentProcessor> SkLumaColorFilter::asFragmentProcessor(GrContext*) const {
    return LumaColorFilterEffect::Make();
}
#endif
