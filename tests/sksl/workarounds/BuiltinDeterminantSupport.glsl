#version 400

float _determinant2(mat2 m) {
return m[0].x*m[1].y - m[0].y*m[1].x;
}
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return _determinant2(testMatrix2x2) == -2.0 ? colorGreen : colorRed;
}
