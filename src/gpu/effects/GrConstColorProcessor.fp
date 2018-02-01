/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

enum class InputMode {
    kIgnore,
    kModulateRGBA,
    kModulateA,

    kLast = kModulateA
};

layout(ctype=GrColor4f) in half4 color;
uniform half4 colorUniform;
layout(ctype=GrColor4f) half4 prevColor;
layout(key) in InputMode mode;

@optimizationFlags {
    OptFlags(color, mode)
}

void main() {
    @switch (mode) {
        case InputMode::kIgnore:
            sk_OutColor = colorUniform;
            break;
        case InputMode::kModulateRGBA:
            sk_OutColor = sk_InColor * colorUniform;
            break;
        case InputMode::kModulateA:
            sk_OutColor = sk_InColor.a * colorUniform;
            break;
    }
}

@setData(pdman) {
    // We use the "illegal" color value as an uninit sentinel. With GrColor4f, the "illegal"
    // color is *really* illegal (not just unpremultiplied), so this check is simple.
    if (prevColor != color) {
        pdman.set4fv(colorUniform, 1, color.fRGBA);
        prevColor = color;
    }
}

@class {
    static const int kInputModeCnt = (int) InputMode::kLast + 1;

    static OptimizationFlags OptFlags(GrColor4f color, InputMode mode) {
        OptimizationFlags flags = kConstantOutputForConstantInput_OptimizationFlag;
        if (mode != InputMode::kIgnore) {
            flags |= kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
        if (color.isOpaque()) {
            flags |= kPreservesOpaqueInput_OptimizationFlag;
        }
        return flags;
    }

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        switch (fMode) {
            case InputMode::kIgnore:
                return fColor;
            case InputMode::kModulateA:
                return fColor.mulByScalar(input.fRGBA[3]);
            case InputMode::kModulateRGBA:
                return fColor.modulate(input);
        }
        SK_ABORT("Unexpected mode");
        return GrColor4f::TransparentBlack();
    }
}

@test(d) {
    GrColor4f color;
    int colorPicker = d->fRandom->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = d->fRandom->nextULessThan(0x100);
            uint32_t r = d->fRandom->nextULessThan(a+1);
            uint32_t g = d->fRandom->nextULessThan(a+1);
            uint32_t b = d->fRandom->nextULessThan(a+1);
            color = GrColor4f::FromGrColor(GrColorPackRGBA(r, g, b, a));
            break;
        }
        case 1:
            color = GrColor4f::TransparentBlack();
            break;
        case 2:
            uint32_t c = d->fRandom->nextULessThan(0x100);
            color = GrColor4f::FromGrColor(c | (c << 8) | (c << 16) | (c << 24));
            break;
    }
    InputMode mode = static_cast<InputMode>(d->fRandom->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Make(color, mode);
}