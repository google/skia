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

in fragmentProcessor? inputFP;
layout(ctype=SkPMColor4f, tracked) in uniform half4 color;
layout(key) in InputMode mode;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (kConstantOutputForConstantInput_OptimizationFlag |
     ((mode != InputMode::kIgnore) ? kCompatibleWithCoverageAsAlpha_OptimizationFlag
                                   : kNone_OptimizationFlags) |
     ((color.isOpaque()) ? kPreservesOpaqueInput_OptimizationFlag
                         : kNone_OptimizationFlags))
}

void main() {
    @switch (mode) {
        case InputMode::kIgnore: {
            sk_OutColor = color;
            break;
        }
        case InputMode::kModulateRGBA: {
            half4 inputColor = sample(inputFP, sk_InColor);
            sk_OutColor = inputColor * color;
            break;
        }
        case InputMode::kModulateA: {
            half inputAlpha = sample(inputFP, sk_InColor).a;
            sk_OutColor = inputAlpha * color;
            break;
        }
    }
}

@class {
    static const int kInputModeCnt = (int) InputMode::kLast + 1;

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        switch (mode) {
            case InputMode::kIgnore: {
                return color;
            }
            case InputMode::kModulateA: {
                SkPMColor4f input = this->numChildProcessors()
                    ? ConstantOutputForConstantInput(this->childProcessor(inputFP_index), inColor)
                    : inColor;
                return color * input.fA;
            }
            case InputMode::kModulateRGBA: {
                SkPMColor4f input = this->numChildProcessors()
                    ? ConstantOutputForConstantInput(this->childProcessor(inputFP_index), inColor)
                    : inColor;
                return color * input;
            }
        }
        SkUNREACHABLE;
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
    return GrConstColorProcessor::Make(/*inputFP=*/nullptr, color, mode);
}
