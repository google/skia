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
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
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

SkColorFilter* SkLumaColorFilter::Create() {
    return SkNEW(SkLumaColorFilter);
}

SkLumaColorFilter::SkLumaColorFilter() : INHERITED() {}

SkFlattenable* SkLumaColorFilter::CreateProc(SkReadBuffer&) {
    return SkNEW(SkLumaColorFilter);
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
    static GrFragmentProcessor* Create() {
        GR_CREATE_STATIC_PROCESSOR(gLumaEffect, LumaColorFilterEffect, ());
        return SkRef(gLumaEffect);
    }

    const char* name() const override { return "Luminance-to-Alpha"; }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GLProcessor, (*this));
    }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        GLProcessor(const GrProcessor&) {}

        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {}

        virtual void emitCode(EmitArgs& args) override {
            if (NULL == args.fInputColor) {
                args.fInputColor = "vec4(1)";
            }

            GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb);\n",
                                   SK_ITU_BT709_LUM_COEFF_R,
                                   SK_ITU_BT709_LUM_COEFF_G,
                                   SK_ITU_BT709_LUM_COEFF_B,
                                   args.fInputColor);
            fsBuilder->codeAppendf("\t%s = vec4(0, 0, 0, luma);\n",
                                   args.fOutputColor);

        }

    private:
        typedef GrGLFragmentProcessor INHERITED;
    };

private:
    LumaColorFilterEffect() {
        this->initClassID<LumaColorFilterEffect>();
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // The output is always black. The alpha value for the color passed in is arbitrary.
        inout->setToOther(kRGB_GrColorComponentFlags, GrColorPackRGBA(0, 0, 0, 0),
                          GrInvariantOutput::kWill_ReadInput);
    }
};

bool SkLumaColorFilter::asFragmentProcessors(GrContext*, GrProcessorDataManager*,
                                             SkTDArray<GrFragmentProcessor*>* array) const {

    GrFragmentProcessor* frag = LumaColorFilterEffect::Create();
    if (frag) {
        if (array) {
            *array->append() = frag;
        } else {
            frag->unref();
            SkDEBUGCODE(frag = NULL;)
        }
        return true;
    }
    return false;
}
#endif
