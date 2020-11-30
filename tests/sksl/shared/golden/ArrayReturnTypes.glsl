
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(float[2][2](float[2](1.0, 2.0), float[2](3.0, 4.0))[0][0], float[2][2](float[2](1.0, 2.0), float[2](3.0, 4.0))[0][1], float[2][2](float[2](1.0, 2.0), float[2](3.0, 4.0))[1][0], float[2][2](float[2](1.0, 2.0), float[2](3.0, 4.0))[1][1]);
}
