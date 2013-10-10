/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLumaColorFilter.h"

#include "SkColorPriv.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "gl/GrGLEffect.h"
#include "GrContext.h"
#include "GrTBackendEffectFactory.h"
#endif

void SkLumaColorFilter::filterSpan(const SkPMColor src[], int count,
                                   SkPMColor dst[]) const {
    const SkUnPreMultiply::Scale* table = SkUnPreMultiply::GetScaleTable();

    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];

        unsigned r = SkGetPackedR32(c);
        unsigned g = SkGetPackedG32(c);
        unsigned b = SkGetPackedB32(c);
        unsigned a = SkGetPackedA32(c);

       // No need to do anything for white (luminance == 1.0)
       if (a != r || a != g || a != b) {
            /*
             *  To avoid un-premultiplying multiple components, we can start
             *  with the luminance computed in PM space:
             *
             *    Lum = i * (r / a) + j * (g / a) + k * (b / a)
             *    Lum = (i * r + j * g + k * b) / a
             *    Lum = Lum'(PM) / a
             *
             *  Then the filter function is:
             *
             *    C' = [ Lum * a, Lum * r, Lum * g, Lum * b ]
             *
             *  which is equivalent to:
             *
             *    C' = [ Lum'(PM), Lum * r, Lum * g, Lum * b ]
             */
            unsigned pm_lum = SkComputeLuminance(r, g, b);
            unsigned lum = SkUnPreMultiply::ApplyScale(table[a], pm_lum);

            c = SkPackARGB32(pm_lum,
                             SkMulDiv255Round(r, lum),
                             SkMulDiv255Round(g, lum),
                             SkMulDiv255Round(b, lum));
        }

        dst[i] = c;
    }
}

SkColorFilter* SkLumaColorFilter::Create() {
    return SkNEW(SkLumaColorFilter);
}

SkLumaColorFilter::SkLumaColorFilter()
    : INHERITED() {
}

SkLumaColorFilter::SkLumaColorFilter(SkFlattenableReadBuffer& buffer)
    : INHERITED(buffer) {
}

void SkLumaColorFilter::flatten(SkFlattenableWriteBuffer&) const {
}

#ifdef SK_DEVELOPER
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
        *validFlags = 0;
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

            // The max() is to guard against 0 / 0 during unpremul when the incoming color is
            // transparent black.
            builder->fsCodeAppendf("\tfloat nonZeroAlpha = max(%s.a, 0.00001);\n", inputColor);
            builder->fsCodeAppendf("\tfloat luma = dot(vec3(%f, %f, %f), %s.rgb);\n",
                                   SK_ITU_BT709_LUM_COEFF_R,
                                   SK_ITU_BT709_LUM_COEFF_G,
                                   SK_ITU_BT709_LUM_COEFF_B,
                                   inputColor);
            builder->fsCodeAppendf("\t%s = vec4(%s.rgb * luma / nonZeroAlpha, luma);\n",
                                   outputColor, inputColor);

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
