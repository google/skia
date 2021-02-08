
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    mat2 _1_m22 = mat3x2(32.0) * mat2x3(23.0);
    _1_m22 *= _1_m22;
    mat3 _2_m33 = mat4x3(44.0) * mat3x4(34.0);
    _2_m33 *= _2_m33;
    mat4 _3_m44 = mat2x4(24.0) * mat4x2(42.0);
    _3_m44 *= _3_m44;
    mat2 _5_m22 = mat3x2(32.0) * mat2x3(23.0);
    _5_m22 *= _5_m22;
    mat3 _6_m33 = mat4x3(44.0) * mat3x4(34.0);
    _6_m33 *= _6_m33;
    mat4 _7_m44 = mat2x4(24.0) * mat4x2(42.0);
    _7_m44 *= _7_m44;
    return colorGreen;


}
