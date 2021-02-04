
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float result = 0.0;
    result += mat2(mat3(1.0))[0].x;
    result += mat2(mat4(1.0))[0].x;
    result += mat3(mat4(1.0))[0].x;
    result += mat3(mat2(1.0))[0].x;
    result += mat4(mat3(mat2(1.0)))[0].x;
    result += mat2(mat3(mat4(1.0)))[0].x;
    return result == 6.0 ? colorGreen : colorRed;
}
