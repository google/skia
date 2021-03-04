
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(float[4](1.0, 2.0, 3.0, 4.0)[0], float[4](1.0, 2.0, 3.0, 4.0)[1u], float[4](1.0, 2.0, 3.0, 4.0)[2], float[4](1.0, 2.0, 3.0, 4.0)[3u]);
}
