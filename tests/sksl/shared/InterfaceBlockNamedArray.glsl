
out vec4 sk_FragColor;
layout (binding = 123) uniform testBlock {
    layout (offset = 0) float s;
    layout (offset = 16) mat2 m;
    layout (offset = 48) float[2] a;
    layout (offset = 80) mat2[2] am;
} test[2];
void main() {
    sk_FragColor = vec4(test[0].s, test[1].m[1].x, test[0].a[1], test[1].am[1][0].y);
}
