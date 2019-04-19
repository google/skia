/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Mixes the output of two FPs.

in fragmentProcessor  fp0;
in fragmentProcessor? fp1;
in uniform half      weight;

@class {

    static OptimizationFlags OptFlags(const std::unique_ptr<GrFragmentProcessor>& fp0,
                                      const std::unique_ptr<GrFragmentProcessor>& fp1) {
        auto flags = ProcessorOptimizationFlags(fp0.get());
        if (fp1) {
            flags &= ProcessorOptimizationFlags(fp1.get());
        }
        return flags;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        const auto c0 = ConstantOutputForConstantInput(this->childProcessor(0), input),
                   c1 = (this->numChildProcessors() > 1)
                      ? ConstantOutputForConstantInput(this->childProcessor(1), input)
                      : input;
        return {
            c0.fR + (c1.fR - c0.fR) * weight,
            c0.fG + (c1.fG - c0.fG) * weight,
            c0.fB + (c1.fB - c0.fB) * weight,
            c0.fA + (c1.fA - c0.fA) * weight
        };
    }
}

@optimizationFlags { OptFlags(fp0, fp1) }

void main() {
    half4 in0 = process(fp0, sk_InColor);
    half4 in1 = (fp1 != null) ? process(fp1, sk_InColor) : sk_InColor;

    sk_OutColor = mix(in0, in1, weight);
}
