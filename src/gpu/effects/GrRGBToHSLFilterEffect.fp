/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

void main() {
    half nonZeroAlpha = max(sk_InColor.a, 0.0001);
    half3 c = sk_InColor.rgb / nonZeroAlpha;

//    half3 c = half3(sk_InColor.r, sk_InColor.g, sk_InColor.b);
    half4 K = half4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    half4 p = mix(half4(c.bg, K.wz), half4(c.gb, K.xy), step(c.b, c.g));
    half4 q = mix(half4(p.xyw, c.r), half4(c.r, p.yzx), step(p.x, c.r));

    half d = q.x - min(q.w, q.y);
    //half e = 1.0e-10;
    half e = 0.0001;
    half3 hsv = half3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);

    sk_OutColor.r = hsv.x;
    sk_OutColor.b = hsv.z * (1 - hsv.y * 0.5);
    half cmin = min(sk_OutColor.b, 1 - sk_OutColor.b);
    sk_OutColor.g = (cmin != 0) ? (hsv.z - sk_OutColor.b) / cmin : 0;
    sk_OutColor.a = sk_InColor.a;
}
