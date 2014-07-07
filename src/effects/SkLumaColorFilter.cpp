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
#include "gl/GrGLEffect.h"
#include "GrContext.h"
#include "GrTBackendEffectFactory.h"
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

SkLumaColorFilter::SkLumaColorFilter()
    : INHERITED() {
}

SkLumaColorFilter::SkLumaColorFilter(SkReadBuffer& buffer)
    : INHERITED(buffer) {
}

void SkLumaColorFilter::flatten(SkWriteBuffer&) const {
}

#ifndef SK_IGNORE_TO_STRING
void SkLumaColorFilter::toString(SkString* str) const {
    str->append("SkLumaColorFilter ");
}
#endif

#if SK_SUPPORT_GPU
class LumaColorFilterEffect : public GrEffect {
public:
    static GrEffectRef* Create() {
        AutoEffectUnref effect(SkNEW(LumaColorFilterEffect));
        return CreateEffectRef(effect);
    }

    static const char* Name() { return "Luminance-to-Alpha"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<LumaColorFilterEffect>::getInstance();
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // The output is always black.
        *color = GrColorPackRGBA(0, 0, 0, GrColorUnpackA(*color));
        *validFlags = kRGB_GrColorComponentFlags;
    }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory,
                 const GrDrawEffect&)
        : INHERITED(factory) {
        }

        static EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&) {
            // this class always generates the same code.
            return 0;
        }

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect&,
                              EffectKey,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray&) SK_OVERRIDE {
            if (NULL == inputColor) {
                inputColor = "vec4(1)";
            }

            builder->fsCodeAppendf("\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb);\n",
                                   SK_ITU_BT709_LUM_COEFF_R,
                                   SK_ITU_BT709_LUM_COEFF_G,
                                   SK_ITU_BT709_LUM_COEFF_B,
                                   inputColor);
            builder->fsCodeAppendf("\t%s = vec4(0, 0, 0, luma);\n",
                                   outputColor);

        }

    private:
        typedef GrGLEffect INHERITED;
    };

private:
    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE {
        return true;
    }
};

GrEffectRef* SkLumaColorFilter::asNewEffect(GrContext*) const {
    return LumaColorFilterEffect::Create();
}
#endif
