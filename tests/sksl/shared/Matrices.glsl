
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_float() {
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
    return test_float() && test_half() ? colorGreen : colorRed;
}
