
out vec4 sk_FragColor;
float a;
void main() {
    sk_FragColor.x = float(isinf(a) ? 1 : 0);
}
