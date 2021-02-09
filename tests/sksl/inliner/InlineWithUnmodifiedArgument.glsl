
out vec4 sk_FragColor;
float basic(float x) {
    return x * 2.0;
}
void main() {
    sk_FragColor.x = basic(1.0);
    float y = 2.0;
    sk_FragColor.y = basic(y);
}
