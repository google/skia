
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float test1[4] = float[4](1.0, 2.0, 3.0, 4.0);
    vec2 test2[2] = vec2[2](vec2(1.0, 2.0), vec2(3.0, 4.0));
    mat4 test3[1] = mat4[1](mat4(16.0));
    return (test1[3] + test2[1].y) + test3[0][3].w == 24.0 ? colorGreen : colorRed;
}
