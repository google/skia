/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? child1;
in fragmentProcessor? child2;
in fragmentProcessor lerp;

void main() {
    sk_OutColor = mix(child1 != null ? sample(child1) : sk_InColor,
                      child2 != null ? sample(child2) : sk_InColor,
                      sample(lerp).r);
}
