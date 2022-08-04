
out vec4 sk_FragColor;
layout (binding = 456) buffer testBlock {
    float x;
} test;
void main() {
    sk_FragColor = vec4(test.x);
}
