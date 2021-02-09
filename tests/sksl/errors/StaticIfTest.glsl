
out vec4 sk_FragColor;
void main() {
    float x = sqrt(25.0);
    float y = 10.0;
    if (x < y) {
        sk_FragColor = vec4(1.0);
    }
}
