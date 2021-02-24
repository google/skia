
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_half() {
    vec3 v1 = mat3(1.0) * vec3(2.0);
    vec3 v2 = vec3(2.0) * mat3(1.0);
    mat2 m1 = mat2(vec4(1.0, 2.0, 3.0, 4.0));
    mat2 m2 = mat2(vec4(0.0));
    mat2 m3 = m1;
    mat2 m4 = mat2(1.0);
    m3 *= m4;
    mat2 m5 = mat2(m1[0].x);
    mat2 m6 = mat2(1.0, 2.0, 3.0, 4.0);
    m6 += m5;
    mat2 m7 = mat2(5.0, vec3(6.0, 7.0, 8.0));
    mat3 m9 = mat3(1.0);
    mat4 m10 = mat4(1.0);
    mat4 m11 = mat4(2.0);
    m11 -= m10;
    return true;
}
vec4 main() {
    bool _0_test_float;
    vec3 _1_v1 = mat3(1.0) * vec3(2.0);
    vec3 _2_v2 = vec3(2.0) * mat3(1.0);
    mat2 _3_m1 = mat2(vec4(1.0, 2.0, 3.0, 4.0));
    mat2 _4_m2 = mat2(vec4(0.0));
    mat2 _5_m3 = _3_m1;
    mat2 _6_m4 = mat2(1.0);
    _5_m3 *= _6_m4;
    mat2 _7_m5 = mat2(_3_m1[0].x);
    mat2 _8_m6 = mat2(1.0, 2.0, 3.0, 4.0);
    _8_m6 += _7_m5;
    mat2 _9_m7 = mat2(5.0, vec3(6.0, 7.0, 8.0));
    mat3 _10_m9 = mat3(1.0);
    mat4 _11_m10 = mat4(1.0);
    mat4 _12_m11 = mat4(2.0);
    _12_m11 -= _11_m10;
    return true && test_half() ? colorGreen : colorRed;

}
