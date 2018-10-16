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

layout(ctype=SkPMColor4f, tracked) in uniform half4 color;
layout(key) in InputMode mode;

@optimizationFlags {
    OptFlags(color, mode)
}

void main() {
    @switch (mode) {
        case InputMode::kIgnore:
            sk_OutColor = color;
            break;
        case InputMode::kModulateRGBA:
            sk_OutColor = sk_InColor * color;
            break;
        case InputMode::kModulateA:
            sk_OutColor = sk_InColor.a * color;
            break;
    }
}

@class {
    static const int kInputModeCnt = (int) InputMode::kLast + 1;

    static OptimizationFlags OptFlags(const SkPMColor4f& color, InputMode mode) {
        OptimizationFlags flags = kConstantOutputForConstantInput_OptimizationFlag;
        if (mode != InputMode::kIgnore) {
            flags |= kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
        if (color.isOpaque()) {
            flags |= kPreservesOpaqueInput_OptimizationFlag;
        }
        return flags;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        switch (fMode) {
            case InputMode::kIgnore:
                return fColor;
            case InputMode::kModulateA:
                return fColor * input.fA;
            case InputMode::kModulateRGBA:
                return fColor * input;
        }
        SK_ABORT("Unexpected mode");
        return SK_PMColor4fTRANSPARENT;
    }
}

@test(d) {
    SkPMColor4f color;
    int colorPicker = d->fRandom->nextULessThan(3);
    switch (colorPicker) {
        case 0: {
            uint32_t a = d->fRandom->nextULessThan(0x100);
            uint32_t r = d->fRandom->nextULessThan(a+1);
            uint32_t g = d->fRandom->nextULessThan(a+1);
            uint32_t b = d->fRandom->nextULessThan(a+1);
            color = SkPMColor4f::FromBytes_RGBA(GrColorPackRGBA(r, g, b, a));
            break;
        }
        case 1:
            color = SK_PMColor4fTRANSPARENT;
            break;
        case 2:
            uint32_t c = d->fRandom->nextULessThan(0x100);
            color = SkPMColor4f::FromBytes_RGBA(c | (c << 8) | (c << 16) | (c << 24));
            break;
    }
    InputMode mode = static_cast<InputMode>(d->fRandom->nextULessThan(kInputModeCnt));
    return GrConstColorProcessor::Make(color, mode);
}
