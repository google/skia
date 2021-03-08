
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float result = 0.0;
    mat2 a = mat2(mat3(1.0));
    result += a[0].x;
    mat2 b = mat2(mat4(1.0));
    result += b[0].x;
    mat3 c = mat3(mat4(1.0));
    result += c[0].x;
    mat3 d = mat3(mat2(1.0));
    result += d[0].x;
    mat4 e = mat4(mat3(mat2(1.0)));
    result += e[0].x;
    mat2 f = mat2(mat3(mat4(1.0)));
    result += f[0].x;
    return result == 6.0 ? colorGreen : colorRed;
}
