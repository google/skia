
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float result = 0.0;
    mat3 g = mat3(mat2x3(1.0));
    result += g[0].x;
    mat3 h = mat3(mat3x2(1.0));
    result += h[0].x;
    mat4 i = mat4(mat4x3((mat4x2(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0) * 1.0)));
    result += i[0].x;
    mat4 j = mat4(mat3x4(mat2x4(1.0)));
    result += j[0].x;
    mat2x4 k = mat2x4((mat4x2(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0) * 1.0));
    result += k[0].x;
    mat4x2 l = mat4x2(mat2x4(1.0));
    result += l[0].x;
    return result == 6.0 ? colorGreen : colorRed;
}
