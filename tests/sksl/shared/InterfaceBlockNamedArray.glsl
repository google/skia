
out vec4 sk_FragColor;
layout (binding = 123) uniform testBlock {
    layout (offset = 0) float x;
    layout (offset = 16) mat2 m;
} test[2];
void main() {
    sk_FragColor = vec4(test[0].x, test[0].m[0].y, test[1].m[1]);
}
