
out vec4 sk_FragColor;
void main() {
    float r, g;
    sqrt(1.0) > 0.0 ? r : g = sqrt(1.0);
    sqrt(0.0) > 0.0 ? r : g = sqrt(0.0);
    sk_FragColor = vec4(r, g, 1.0, 1.0);
}
