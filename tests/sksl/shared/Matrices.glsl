
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    mat2 _0_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _0_m3 *= mat2(1.0);
    mat2 _1_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _2_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _2_m6 += _1_m5;
    mat4 _3_m11 = mat4(2.0);
    _3_m11 -= mat4(1.0);
    mat2 _4_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _4_m3 *= mat2(1.0);
    mat2 _5_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _6_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _6_m6 += _5_m5;
    mat4 _7_m11 = mat4(2.0);
    _7_m11 -= mat4(1.0);
    return colorGreen;


}
