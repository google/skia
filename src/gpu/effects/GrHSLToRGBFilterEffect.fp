/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

void main() {
    half3   hsl = sk_InColor.rgb;
    half3 kBias = half3(0, 2.0/3, 1.0/3);
    half3     p = abs(fract(hsl.xxx + kBias.xyz) * 6 - 3);
    half3   rgb = saturate(p - 1);
    half      C = (1 - abs(2 * hsl.z - 1)) * hsl.y;

    sk_OutColor.rgb = (rgb - 0.5) * C + hsl.z;
    sk_OutColor.a   = sk_InColor.a;

    sk_OutColor = saturate(sk_OutColor);
    sk_OutColor.rgb *= sk_OutColor.a;
}
