/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor child1;
in fragmentProcessor child2;

void main() {
    // #1
    sk_OutColor = sample(child1);

    // #2
//    sk_OutColor = sample(child1, sk_TransformedCoords2D[0] / 2);

    // #3
//    sk_OutColor = sample(child1, sample(child2).xy);

    // #4
//    sk_OutColor = sample(child1, sample(child2, sk_TransformedCoords2D[0] / 2).xy * 2);

    // #5
//    int count = int(sample(child1, sk_TransformedCoords2D[0] / 2).r * 4);
//    half4 result = half4(0, 0, 0, 0);
//    for (int i = 0; i < count; ++i) {
//        result += sample(child2, sk_TransformedCoords2D[0] + i);
//    }
//    sk_OutColor = result;
}
