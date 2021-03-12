
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half() {
    vec3 v1 = mat3(1.0) * vec3(2.0);
    vec3 v2 = vec3(2.0) * mat3(1.0);
    mat2 m1 = mat2(1.0, 2.0, 3.0, 4.0);
    mat2 m2 = mat2(vec4(0.0));
    mat2 m3 = m1;
    mat2 m4 = mat2(1.0);
    m3 *= m4;
    mat2 m5 = mat2(m1[0].x);
    mat2 m6 = mat2(1.0, 2.0, 3.0, 4.0);
    m6 += m5;
    mat2 m7 = mat2(5.0, 6.0, 7.0, 8.0);
    mat3 m9 = mat3(1.0);
    mat4 m10 = mat4(1.0);
    mat4 m11 = mat4(2.0);
    m11 -= m10;
    return true;
}
vec4 main() {
    vec3 _0_v1 = mat3(1.0) * vec3(2.0);
    vec3 _1_v2 = vec3(2.0) * mat3(1.0);
    mat2 _2_m1 = mat2(1.0, 2.0, 3.0, 4.0);
    mat2 _3_m2 = mat2(vec4(0.0));
    mat2 _4_m3 = _2_m1;
    mat2 _5_m4 = mat2(1.0);
    _4_m3 *= _5_m4;
    mat2 _6_m5 = mat2(_2_m1[0].x);
    mat2 _7_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _7_m6 += _6_m5;
    mat2 _8_m7 = mat2(5.0, 6.0, 7.0, 8.0);
    mat3 _9_m9 = mat3(1.0);
    mat4 _10_m10 = mat4(1.0);
    mat4 _11_m11 = mat4(2.0);
    _11_m11 -= _10_m10;
    return true && test_half() ? colorGreen : colorRed;
}
