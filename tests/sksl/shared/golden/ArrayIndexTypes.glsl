
out vec4 sk_FragColor;
void main() {
    float array[4];
    array[0] = 0.0;
    array[1] = 1.0;
    array[2] = 2.0;
    array[3] = 3.0;
    sk_FragColor = vec4(array[0], array[1], array[2], array[3u]);
}
