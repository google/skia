/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Ignores its own input color and invokes 'fp' with a constant color
// The constant color can either be specified as a literal or as a
// uniform, controlled by useUniform.

in fragmentProcessor fp;
layout(key) in bool useUniform;
layout(when=useUniform, ctype=SkPMColor4f) in uniform half4 uniformColor;
layout(when=!useUniform, key, ctype=SkPMColor4f) in half4 literalColor;

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                     const SkPMColor4f& color,
                                                     bool useUniform = true) {
        return std::unique_ptr<GrFragmentProcessor>(
            new GrOverrideInputFragmentProcessor(std::move(fp), useUniform, color, color));
    }
}

@class {
    static OptimizationFlags OptFlags(const std::unique_ptr<GrFragmentProcessor>& fp) {
        return ProcessorOptimizationFlags(fp.get());
    }

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        return ConstantOutputForConstantInput(this->childProcessor(0), uniformColor);
    }
}

@optimizationFlags { OptFlags(fp) }

void main() {
    half4 constColor;
    @if(useUniform) {
        constColor = uniformColor;
    } else {
        constColor = literalColor;
    }
    sk_OutColor = process(fp, constColor);
}
