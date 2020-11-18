/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

in fragmentProcessor fp;

half4 main() {
    return sample(fp, sk_FragCoord.xy);
}

@optimizationFlags {
    ProcessorOptimizationFlags(fp.get())
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        return ConstantOutputForConstantInput(this->childProcessor(0), inColor);
    }
}

@make{
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp) {
        return std::unique_ptr<GrFragmentProcessor>(new GrDeviceSpaceEffect(std::move(fp)));
    }
}

@test(d) {
    return GrDeviceSpaceEffect::Make(GrProcessorUnitTest::MakeChildFP(d));
}
