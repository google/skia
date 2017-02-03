/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkHighContrastFilter.h"

#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"

namespace {

SkScalar Hue2RGB(SkScalar p, SkScalar q, SkScalar t) {
    if (t < 0) {
        t += 1;
    } else if (t > 1) {
        t -= 1;
    }

    if (t < 1/6.f) {
        return p + (q - p) * 6 * t;
    }

    if (t < 1/2.f) {
        return q;
    }

    if (t < 2/3.f) {
        return p + (q - p) * (2/3.f - t) * 6;
    }

    return p;
}

unsigned char SkScalarToCharClamp(SkScalar f) {
    if (f <= 0) {
        return 0;
    } else if (f >= 1) {
        return 255;
    }
    return static_cast<unsigned char>(255 * f);
}

// Sigmoid-based contrast
SkScalar IncreaseContrast(SkScalar f, SkScalar contrast) {
    SkScalar v = 1 / (1 + exp(-20 * contrast * (f - 0.5f)));

    // Normalize so that f(0) = 0 and f(1) = 1
    SkScalar z = 1 / (1 + exp(10 * contrast));
    return (v - z) / (1 - 2 * z);
}

}  // namespace

void SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                 unsigned char* r,
                                 unsigned char* g,
                                 unsigned char* b) {
    // Convert to HSL
    SkScalar rf = *r / 255.f;
    SkScalar gf = *g / 255.f;
    SkScalar bf = *b / 255.f;
    SkScalar max = SkTMax(SkTMax(rf, gf), bf);
    SkScalar min = SkTMin(SkTMin(rf, gf), bf);
    SkScalar l = (max + min) / 2;
    SkScalar h, s;

    if (max == min || config.fGrayscale ||
        config.fInvertStyle != HighContrastInvertStyle::kInvertLightness) {
        h = 0;
        s = 0;
    } else {
        SkScalar d = max - min;
        s = l > 0.5f ? d / (2 - max - min) : d / (max + min);
        if (max == rf) {
            h = (gf - bf) / d + (gf < bf ? 6 : 0);
        } else if (max == gf) {
            h = (bf - rf) / d + 2;
        } else {
            h = (rf - gf) / d + 4;
        }
        h /= 6;
    }

    if (config.fInvertStyle == HighContrastInvertStyle::kInvertBrightness) {
        if (config.fGrayscale) {
            rf = 1.0f - l;
            gf = 1.0f - l;
            bf = 1.0f - l;
        } else {
            rf = 1.0f - rf;
            gf = 1.0f - gf;
            bf = 1.0f - bf;
        }

        if (config.fExponent > 0.0f) {
            rf = pow(rf, config.fExponent);
            gf = pow(gf, config.fExponent);
            bf = pow(bf, config.fExponent);
        }
    } else if (config.fInvertStyle ==
                   HighContrastInvertStyle::kInvertLightness) {
        l = 1.0f - l;
        if (config.fExponent > 0.0f) {
            l = pow(l, config.fExponent);
        }

        if (config.fContrast > 0.0f) {
            l = IncreaseContrast(l, config.fContrast);
        }

        if (s == 0) {
            // Grayscale
            rf = l;
            gf = l;
            bf = l;
        } else {
            SkScalar q = l < 0.5f ? l * (1 + s) : l + s - l * s;
            SkScalar p = 2 * l - q;
            rf = Hue2RGB(p, q, h + 1/3.f);
            gf = Hue2RGB(p, q, h);
            bf = Hue2RGB(p, q, h - 1/3.f);
        }
    } else if (config.fGrayscale) {
        if (config.fExponent > 0.0f) {
            l = pow(l, config.fExponent);
        }
        rf = l;
        gf = l;
        bf = l;
    } else if (config.fExponent > 0.0f) {
        rf = pow(rf, config.fExponent);
        gf = pow(gf, config.fExponent);
        bf = pow(bf, config.fExponent);
    }

    if (config.fContrast != 0.0f &&
        config.fInvertStyle != HighContrastInvertStyle::kInvertLightness) {
        rf = IncreaseContrast(rf, config.fContrast);
        gf = IncreaseContrast(gf, config.fContrast);
        bf = IncreaseContrast(bf, config.fContrast);
    }

    *r = SkScalarToCharClamp(rf);
    *g = SkScalarToCharClamp(gf);
    *b = SkScalarToCharClamp(bf);
}

SkColor SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                    SkColor srcColor) {
    unsigned char a = SkColorGetA(srcColor);
    unsigned char r = SkColorGetR(srcColor);
    unsigned char g = SkColorGetG(srcColor);
    unsigned char b = SkColorGetB(srcColor);
    SkHighContrastFilter::Apply(config, &r, &g, &b);
    return SkColorSetARGB(a, r, g, b);
}

SkPaint SkHighContrastFilter::Apply(const SkHighContrastConfig& config,
                                    const SkPaint& src) {
  if (src.getShader()) {
    fprintf(stderr, "Paint has shader");
  }

  SkPaint dst = src;
  dst.setColor(SkHighContrastFilter::Apply(config, dst.getColor()));
  return dst;
}

class SkHighContrast_Filter : public SkColorFilter {
public:
    SkHighContrast_Filter(const SkHighContrastConfig& config) {
        fConfig = config;
    }

    virtual ~SkHighContrast_Filter() { }

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const
          override;
    bool onAppendStages(SkRasterPipeline* p,
                        SkColorSpace* dst,
                        SkArenaAlloc* scratch,
                        bool shaderIsOpaque) const override;

    SK_TO_STRING_OVERRIDE()

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkHighContrast_Filter)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkHighContrastConfig fConfig;

    friend class SkHighContrastFilter;

    typedef SkColorFilter INHERITED;
};

void SkHighContrast_Filter::filterSpan(const SkPMColor src[], int count,
                                       SkPMColor dst[]) const {
    for (int i = 0; i < count; ++i) {
        dst[i] = src[i];
        SkPMColor c = src[i];
        unsigned char a = SkGetPackedA32(c);
        unsigned char r = SkGetPackedR32(c);
        unsigned char g = SkGetPackedG32(c);
        unsigned char b = SkGetPackedB32(c);
        SkHighContrastFilter::Apply(fConfig, &r, &g, &b);
        dst[i] = SkPremultiplyARGBInline(a, r, g, b);
    }
}

bool SkHighContrast_Filter::onAppendStages(SkRasterPipeline* p,
                                           SkColorSpace* dst,
                                           SkArenaAlloc* scratch,
                                           bool shaderIsOpaque) const {
  fprintf(stderr, "onAppendStages\n");
    p->append(SkRasterPipeline::rgb_to_hsl);
    return true;
}

void SkHighContrast_Filter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeBool(fConfig.fGrayscale);
    buffer.writeInt(static_cast<int>(fConfig.fInvertStyle));
    buffer.writeScalar(fConfig.fExponent);
    buffer.writeScalar(fConfig.fContrast);
}

sk_sp<SkFlattenable> SkHighContrast_Filter::CreateProc(SkReadBuffer& buffer) {
    SkHighContrastConfig config;
    config.fGrayscale = buffer.readBool();
    config.fInvertStyle = static_cast<HighContrastInvertStyle>(
        buffer.readInt());
    config.fExponent = buffer.readScalar();
    config.fContrast = buffer.readScalar();
    return SkHighContrastFilter::Make(config);
}

sk_sp<SkColorFilter> SkHighContrastFilter::Make(
    const SkHighContrastConfig& config) {
    return sk_make_sp<SkHighContrast_Filter>(config);
}

#ifndef SK_IGNORE_TO_STRING
void SkHighContrast_Filter::toString(SkString* str) const {
    str->append("SkHighContrastColorFilter ");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkHighContrastFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkHighContrast_Filter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
