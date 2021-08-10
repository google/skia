#version 400
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 colorGreen;
uniform mediump vec4 colorRed;
mediump vec4 main() {
    highp float f[4] = float[4](1.0, 2.0, 3.0, 4.0);
    mediump float h[4] = f;
    f = h;
    h = f;
    highp ivec3 i3[3] = ivec3[3](ivec3(1), ivec3(2), ivec3(3));
    mediump ivec3 s3[3] = i3;
    i3 = s3;
    s3 = i3;
    mediump mat2 h2x2[2] = mat2[2](mat2(1.0, 2.0, 3.0, 4.0), mat2(5.0, 6.0, 7.0, 8.0));
    highp mat2 f2x2[2] = h2x2;
    f2x2 = h2x2;
    h2x2 = f2x2;
    return (f == h && i3 == s3) && f2x2 == h2x2 ? colorGreen : colorRed;
}
