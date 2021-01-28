
out vec4 sk_FragColor;
layout (binding = 0) uniform testBlock {
    layout (offset = 0) float x;
    layout (offset = 4) int w;
    layout (offset = 16) float[2] y;
    layout (offset = 48) mat3 z;
};
void main() {
    sk_FragColor = vec4(x, y[0], y[1], 0.0);
}
