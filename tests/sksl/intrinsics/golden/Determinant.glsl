
mat4 _determinant4(mat4 m) {    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];    float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];    float b00 = a00 * a11 - a01 * a10;    float b01 = a00 * a12 - a02 * a10;    float b02 = a00 * a13 - a03 * a10;    float b03 = a01 * a12 - a02 * a11;    float b04 = a01 * a13 - a03 * a11;    float b05 = a02 * a13 - a03 * a12;    float b06 = a20 * a31 - a21 * a30;    float b07 = a20 * a32 - a22 * a30;    float b08 = a20 * a33 - a23 * a30;    float b09 = a21 * a32 - a22 * a31;    float b10 = a21 * a33 - a23 * a31;    float b11 = a22 * a33 - a23 * a32;    return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;}out vec4 sk_FragColor;
mat4 a;
void main() {
    sk_FragColor.x = _determinant4(a);
}
