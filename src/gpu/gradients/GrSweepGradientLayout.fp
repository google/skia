/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Needs a center, bias and scale (or t0 to t1?, probably bias and scale to match the members of SkSweepGradient)

in half4x4 gradientMatrix;

in half bias;
in half scale;

half prevBias;
half prevScale;

uniform half2 uBiasAndScale;

@coordTransform {
    gradientMatrix
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(
        const SkMatrix& inverseLocal,
        const SkPoint& center, SkScalar bias, SkScalar scale);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrSweepGradientLayout::Make(
            const SkMatrix& inverseLocal, const SkPoint& center,
            SkScalar bias, SkScalar scale) {
        SkMatrix matrix = inverseLocal;
        matrix.postConcat(SkMatrix::MakeTrans(-center.fX, -center.fY));
        return std::unique_ptr<GrFragmentProcessor>(new GrSweepGradientLayout(matrix, bias, scale));
    }
}

@setData(pdman) {
    if (bias != prevBias || scale != prevScale) {
        pdman.set2f(uBiasAndScale, bias, scale);
        prevBias = bias;
        prevScale = scale;
    }
}

void main() {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in
    // (sqrt(x^2 + y^2) + x) as the second parameter to atan2 in these cases. We let the device
    // handle the undefined behavior of the second paramenter being 0 instead of doing the
    // divide ourselves and using atan instead.
    half angle;
    if (sk_Caps.atan2ImplementedAsAtanYOverX) {
        angle = 2 * atan(-sk_TransformedCoords2D[0].y, length(sk_TransformedCoords2D[0]) - sk_TransformedCoords2D[0].x);
    } else {
        angle = atan(-sk_TransformedCoords2D[0].y, -sk_TransformedCoords2D[0].x);
    }

    // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
    sk_OutColor = half4((angle * 0.1591549430918 + 0.5 + uBiasAndScale.x) * uBiasAndScale.y);
}
