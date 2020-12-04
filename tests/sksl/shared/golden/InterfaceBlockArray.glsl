
out vec4 sk_FragColor;
layout (binding = 123) uniform testBlock {
    float x;
} test[2];
void main() {
    sk_FragColor = vec4(test[1].x);
}
