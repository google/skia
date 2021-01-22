
out vec4 sk_FragColor;
layout (binding = 456, push_constant) uniform testBlock {
    layout (offset = 16) mat2 m1;
    layout (offset = 32) mat2 m2;
};
void main() {
    sk_FragColor = vec4(m1[0].x, m1[1].y, m2[0].x, m2[1].y);
}
