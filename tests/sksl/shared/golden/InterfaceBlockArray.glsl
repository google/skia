
out vec4 sk_FragColor;
uniform testBlock {
    float x;
} test[2];
void main() {
    sk_FragColor = vec4(test[1].x);
}
