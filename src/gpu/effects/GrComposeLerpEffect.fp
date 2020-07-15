/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? child1;
in fragmentProcessor? child2;
in uniform float weight;

void main() {
    sk_OutColor = mix(sample(child1), sample(child2), half(weight));
}
