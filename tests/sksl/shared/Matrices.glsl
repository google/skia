
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    mat2 _1_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _1_m3 *= mat2(1.0);
    mat2 _2_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _3_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _3_m6 += _2_m5;
    mat4 _4_m11 = mat4(2.0);
    _4_m11 -= mat4(1.0);
    mat2 _6_m3 = mat2(1.0, 2.0, 3.0, 4.0);
    _6_m3 *= mat2(1.0);
    mat2 _7_m5 = mat2(mat2(1.0, 2.0, 3.0, 4.0)[0].x);
    mat2 _8_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _8_m6 += _7_m5;
    mat4 _9_m11 = mat4(2.0);
    _9_m11 -= mat4(1.0);
    return colorGreen;


}
