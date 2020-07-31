/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Should have height = 1px, horizontal axis represents t = 0 to 1
in fragmentProcessor textureFP;

void main(float2 coord) {
    sk_OutColor = sample(textureFP);
}
