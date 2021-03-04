
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float result = 0.0;
    result += mat3(mat2x3(1.0))[0].x;
    result += mat3(mat3x2(1.0))[0].x;
    result += mat4(mat4x3(mat4x2(1.0)))[0].x;
    result += mat4(mat3x4(mat2x4(1.0)))[0].x;
    result += mat2x4(mat4x2(1.0))[0].x;
    result += mat4x2(mat2x4(1.0))[0].x;
    return result == 6.0 ? colorGreen : colorRed;
}
