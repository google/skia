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
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"
#include "GrContext.h"
#include "GrTBackendProcessorFactory.h"
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

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkLumaColorFilter::SkLumaColorFilter(SkReadBuffer& buffer) : INHERITED(buffer) {}
#endif

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
        GR_CREATE_STATIC_FRAGMENT_PROCESSOR(gLumaEffect, LumaColorFilterEffect, ());
        return SkRef(gLumaEffect);
    }

    static const char* Name() { return "Luminance-to-Alpha"; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendFragmentProcessorFactory<LumaColorFilterEffect>::getInstance();
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // The output is always black.
        *color = GrColorPackRGBA(0, 0, 0, GrColorUnpackA(*color));
        *validFlags = kRGB_GrColorComponentFlags;
    }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory,
                    const GrProcessor&)
        : INHERITED(factory) {
        }

        static void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder* b) {}

        virtual void emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor&,
                              const GrProcessorKey&,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray&) SK_OVERRIDE {
            if (NULL == inputColor) {
                inputColor = "vec4(1)";
            }

            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb);\n",
                                   SK_ITU_BT709_LUM_COEFF_R,
                                   SK_ITU_BT709_LUM_COEFF_G,
                                   SK_ITU_BT709_LUM_COEFF_B,
                                   inputColor);
            fsBuilder->codeAppendf("\t%s = vec4(0, 0, 0, luma);\n",
                                   outputColor);

        }

    private:
        typedef GrGLFragmentProcessor INHERITED;
    };

private:
    virtual bool onIsEqual(const GrProcessor&) const SK_OVERRIDE {
        return true;
    }
};

GrFragmentProcessor* SkLumaColorFilter::asFragmentProcessor(GrContext*) const {
    return LumaColorFilterEffect::Create();
}
#endif
