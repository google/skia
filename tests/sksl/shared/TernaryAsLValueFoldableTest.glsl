
out vec4 sk_FragColor;
void main() {
    float r;
    float g;

    r = sqrt(1.0);
    g = sqrt(0.0);
    sk_FragColor.x = r;
    sk_FragColor.y = g;
}
