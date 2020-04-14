/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in uniform float3x3 matrix;
in fragmentProcessor child;

@class {
    static std::unique_ptr<GrFragmentProcessor> Apply(const SkMatrix& matrix,
                                                      std::unique_ptr<GrFragmentProcessor> processor) {
        if (matrix.isIdentity()) {
            return processor;
        }
        SkASSERT(!processor->isSampledWithExplicitCoords());
        SkASSERT(processor->sampleMatrix().fKind == SkSL::SampleMatrix::Kind::kNone);
        return Make(matrix, std::move(processor));
    }    
}

void main() {
    sk_OutColor = sample(child, sk_InColor, matrix);
}
