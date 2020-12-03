
out vec4 sk_FragColor;
layout (binding = 456) uniform testBlock {
    float x;
} test;
void main() {
    sk_FragColor = vec4(test.x);
}
