
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return (float[4](1.0, 2.0, 3.0, 4.0)[3] + vec2[2](vec2(1.0, 2.0), vec2(3.0, 4.0))[1].y) + mat4[1](mat4(16.0))[0][3].w == 24.0 ? colorGreen : colorRed;
}
