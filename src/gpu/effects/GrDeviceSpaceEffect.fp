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

@test(d) {
    std::unique_ptr<GrFragmentProcessor> fp;
    // We have a restriction that explicit coords only work for FPs with zero or one
    // coord transform.
    do {
        fp = GrProcessorUnitTest::MakeChildFP(d);
    } while (fp->numCoordTransforms() > 1);
    return GrDeviceSpaceEffect::Make(std::move(fp));
}
