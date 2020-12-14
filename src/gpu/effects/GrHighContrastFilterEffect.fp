/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@header {
    #include "include/effects/SkHighContrastFilter.h"
}

in fragmentProcessor? inputFP;
in uniform half contrastMod;
layout(key) in bool hasContrast;
layout(key) in bool grayscale;
// invertBrightness and invertLightness are intended to be mutually exclusive.
layout(key) in bool invertBrightness;
layout(key) in bool invertLightness;
layout(key) in bool linearize;

half HSLToRGB(half p, half q, half t) {
    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    return (t < 1/6.) ? p + (q - p) * 6 * t :
           (t < 3/6.) ? q :
           (t < 4/6.) ? p + (q - p) * (2/3. - t) * 6 :
                        p;
}

half4 main() {
    const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);

    half4 inColor = sample(inputFP);
    half4 color = unpremul(inColor);

    @if (linearize) {
        // We approximate the transfer function as gamma 2.0.
        color.rgb = color.rgb * color.rgb;
    }

    @if (grayscale) {
        color = half4(half3(dot(color.rgb, SK_ITU_BT709_LUM_COEFF)), 0);
    }

    @if (invertBrightness) {
        color = half4(1) - color;
    }

    @if (invertLightness) {
        half fmax = max(color.r, max(color.g, color.b));
        half fmin = min(color.r, min(color.g, color.b));
        half l = fmax + fmin;
        half h;
        half s;

        if (fmax == fmin) {
            h = 0;
            s = 0;
        } else {
            half d = fmax - fmin;
            s = (l > 1) ? d / (2 - l)  : d / l;

            // We'd like to just write "if (color.r == fmax) { ... }". On many GPUs, running the
            // angle_d3d9_es2 config, that failed. It seems that max(x, y) is not necessarily equal
            // to either x or y. Tried several ways to fix it, but this was the only reasonable fix.
            if (color.r >= color.g && color.r >= color.b) {
                h = (color.g - color.b) / d + (color.g < color.b ? 6 : 0);
            } else if (color.g >= color.b) {
                h = (color.b - color.r) / d + 2;
            } else {
                h = (color.r - color.g) / d + 4;
            }
            h *= 1/6.;
        }

        l = 1.0 + (l * -0.5);

        if (s == 0) {
            color = half4(l, l, l, 0);
        } else {
            half q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            half p = 2 * l - q;
            color.r = HSLToRGB(p, q, h + 1/3.);
            color.g = HSLToRGB(p, q, h);
            color.b = HSLToRGB(p, q, h - 1/3.);
        }
    }

    @if (hasContrast) {
        half off = (-0.5 * contrastMod) + 0.5;
        color = contrastMod * color + off;
    }

    color = saturate(color);

    @if (linearize) {
        color.rgb = sqrt(color.rgb);
    }

    return color.rgb1 * inColor.a;
}

@optimizationFlags {
    kNone_OptimizationFlags
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     const SkHighContrastConfig& config,
                                                     bool linearize) {
        float contrastMod = (1 + config.fContrast) / (1 - config.fContrast);

        return std::unique_ptr<GrFragmentProcessor>(new GrHighContrastFilterEffect(
                std::move(inputFP),
                contrastMod,
                contrastMod != 1.0,
                config.fGrayscale,
                config.fInvertStyle == SkHighContrastConfig::InvertStyle::kInvertBrightness,
                config.fInvertStyle == SkHighContrastConfig::InvertStyle::kInvertLightness,
                linearize));
    }
}

@test(d) {
    using InvertStyle = SkHighContrastConfig::InvertStyle;
    SkHighContrastConfig config{/*grayscale=*/d->fRandom->nextBool(),
                                InvertStyle(d->fRandom->nextRangeU(0, int(InvertStyle::kLast))),
                                /*contrast=*/d->fRandom->nextF()};
    return GrHighContrastFilterEffect::Make(d->inputFP(), config,
                                            /*linearize=*/d->fRandom->nextBool());
}
