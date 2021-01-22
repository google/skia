
out vec4 sk_FragColor;
void main() {
    float r;
    float g;

    r = sqrt(1.0);
    g = sqrt(0.0);
    sk_FragColor = vec4(r, g, 1.0, 1.0);
}
