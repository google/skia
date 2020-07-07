/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Mixes the output of two FPs.

in fragmentProcessor? inputFP;
in fragmentProcessor  fp0;
in fragmentProcessor? fp1;
in uniform half       weight;

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& in) const override {
        const SkPMColor4f inColor = (inputFP_index >= 0)
              ? ConstantOutputForConstantInput(this->childProcessor(inputFP_index), in)
              : in;
        const SkPMColor4f c0 =
                ConstantOutputForConstantInput(this->childProcessor(fp0_index), inColor);
        const SkPMColor4f c1 = (fp1_index >= 0)
              ? ConstantOutputForConstantInput(this->childProcessor(fp1_index), inColor)
              : inColor;
        return {
            c0.fR + (c1.fR - c0.fR) * weight,
            c0.fG + (c1.fG - c0.fG) * weight,
            c0.fB + (c1.fB - c0.fB) * weight,
            c0.fA + (c1.fA - c0.fA) * weight
        };
    }
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (fp1 ?     ProcessorOptimizationFlags(fp1.get())     : kAll_OptimizationFlags) &
     ProcessorOptimizationFlags(fp0.get())
}

void main() {
    half4 inColor = sample(inputFP, sk_InColor);
    sk_OutColor = mix(sample(fp0, inColor), sample(fp1, inColor), weight);
}
