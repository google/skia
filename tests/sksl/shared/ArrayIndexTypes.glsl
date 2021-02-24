
out vec4 sk_FragColor;
void main() {
    float array[4] = float[4](1.0, 2.0, 3.0, 4.0);
    int x = 0;
    uint y = 1u;
    int z = 2;
    uint w = 3u;
    sk_FragColor = vec4(array[x], array[y], array[z], array[w]);
}
