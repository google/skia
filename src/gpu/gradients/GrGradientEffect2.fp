/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor colorizer;
in fragmentProcessor gradLayout;

void main() {
    half4 t = process(gradLayout);
    // FIXME handle clamping of t
    // FIXME how do we pass the clamped t as the "sk_InColor" to colorizer?
    // look at source code in sksl directory to see how that sorts out
    sk_OutColor = process(colorizer);
}
