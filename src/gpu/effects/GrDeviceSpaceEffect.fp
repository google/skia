/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

in fragmentProcessor fp;

in uniform float3x3 matrix;

void main() {
    float3 p = matrix * (sk_FragCoord.xy1);
    sk_OutColor = sample(fp, sk_InColor, p.xy / p.z);
}

@make{
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> fp,
                                                     const SkMatrix& matrix = SkMatrix::I()) {
        return std::unique_ptr<GrFragmentProcessor>(new GrDeviceSpaceEffect(std::move(fp), matrix));
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
