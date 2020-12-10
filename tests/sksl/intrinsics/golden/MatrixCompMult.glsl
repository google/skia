
out vec4 sk_FragColor;
mat3 a;
mat3 b;
mat4 c;
mat4 d;
void main() {
    sk_FragColor.xyz = matrixCompMult(a, b)[0];
    sk_FragColor = matrixCompMult(c, d)[0];
}
