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
        auto get_flags = [](const std::unique_ptr<GrFragmentProcessor>& fp) {
            auto flags = kNone_OptimizationFlags;

            if (fp->compatibleWithCoverageAsAlpha()) {
                flags |= kCompatibleWithCoverageAsAlpha_OptimizationFlag;
            }

            if (fp->preservesOpaqueInput()) {
                flags |= kPreservesOpaqueInput_OptimizationFlag;
            }

            if (fp->hasConstantOutputForConstantInput()) {
                flags |= kConstantOutputForConstantInput_OptimizationFlag;
            }

            return flags;
        };

        const auto fp0_flags = get_flags(fp0);

        return fp1 ? (fp0_flags & get_flags(fp1)) : fp0_flags;
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        const auto c0 = ConstantOutputForConstantInput(this->childProcessor(0), input),
                   c1 = (this->numChildProcessors() > 1)
                      ? ConstantOutputForConstantInput(this->childProcessor(1), input)
                      : input;
        return {
            c0.fR + (c1.fR - c0.fR) * fWeight,
            c0.fG + (c1.fG - c0.fG) * fWeight,
            c0.fB + (c1.fB - c0.fB) * fWeight,
            c0.fA + (c1.fA - c0.fA) * fWeight
        };
    }
}

@optimizationFlags { OptFlags(fp0, fp1) }

void main() {
    half4 in0 = process(fp0, sk_InColor);
    half4 in1 = (fp1 != null) ? process(fp1, sk_InColor) : sk_InColor;

    sk_OutColor = mix(in0, in1, weight);
}
