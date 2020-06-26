/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

in fragmentProcessor fp;

void main() {
    sk_OutColor = sample(fp, sk_InColor, sk_FragCoord.xy);
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
    std::unique_ptr<GrFragmentProcessor> fp;
    // We have a restriction that explicit coords only work for FPs with zero or one
    // coord transform.
    do {
        fp = GrProcessorUnitTest::MakeChildFP(d);
    } while (fp->numCoordTransforms() > 1);
    return GrDeviceSpaceEffect::Make(std::move(fp));
}
