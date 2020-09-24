
out vec4 sk_FragColor;
uniform testBlock {
    float x;
    float[2] y;
    layout (binding = 12) mat3x2 z;
    bool w;
};
void main() {
    sk_FragColor = vec4(x, y[0], y[1], 0.0);
}
