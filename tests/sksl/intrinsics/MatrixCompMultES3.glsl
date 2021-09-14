
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    mat2x4 h24 = matrixCompMult(mat2x4(9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0), mat2x4(colorRed, colorGreen));
    mat4x2 h42 = matrixCompMult(mat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), mat4x2(colorRed, colorGreen));
    mat4x3 f43 = mat4x3(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    return (h24 == mat2x4(9.0, 0.0, 0.0, 9.0, 0.0, 9.0, 0.0, 9.0) && h42 == mat4x2(1.0, 0.0, 0.0, 4.0, 0.0, 6.0, 0.0, 8.0)) && f43 == mat4x3(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0) ? colorGreen : colorRed;
}
