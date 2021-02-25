/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;

half4 main() {
    half inputAlpha = sample(inputFP).a;
    half factor = 1.0 - inputAlpha;
    factor = half(exp(-factor * factor * 4.0) - 0.018);
    return half4(factor);
}
