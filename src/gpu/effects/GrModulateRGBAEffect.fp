/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? inputFP;
layout(ctype=SkPMColor4f, tracked) in uniform half4 color;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
            (kConstantOutputForConstantInput_OptimizationFlag |
             kCompatibleWithCoverageAsAlpha_OptimizationFlag |
             (color.isOpaque() ? kPreservesOpaqueInput_OptimizationFlag : kNone_OptimizationFlags))
}

void main() {
    sk_OutColor = color * sample(inputFP, sk_InColor);
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        SkPMColor4f input = this->numChildProcessors()
            ? ConstantOutputForConstantInput(this->childProcessor(inputFP_index), inColor)
            : inColor;
        return color * input;
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
    return GrModulateRGBAEffect::Make(/*inputFP=*/nullptr, color);
}
