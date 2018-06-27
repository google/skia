/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in float k1;
in float k2;
in float k3;
in float k4;
layout(key) in bool enforcePMColor;
in fragmentProcessor child;

uniform float4 k;

void main() {
    half4 dst = process(child);
    sk_OutColor = clamp(k.x * sk_InColor * dst + k.y * sk_InColor + k.z * dst + k.w, 0, 1);
    if (enforcePMColor) {
        sk_OutColor.rgb = min(sk_OutColor.rgb, sk_OutColor.a);
    }
}

@setData(pdman) {
    pdman.set4f(k, k1, k2, k3, k4);
}
