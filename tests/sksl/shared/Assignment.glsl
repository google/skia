
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    int ai[1];
    ai[0] = 0;
    ivec4 ai4[1];
    ai4[0] = ivec4(1, 2, 3, 4);
    mat3 ah2x4[1];
    ah2x4[0] = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    vec4 af4[1];
    af4[0].x = 0.0;
    af4[0].ywxz = vec4(1.0);
    ai[0] += ai4[0].x;
    af4[0] *= ah2x4[0][0].x;
    return colorGreen;
}
