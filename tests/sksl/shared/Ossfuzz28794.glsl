
out vec4 sk_FragColor;
void main() {
    int i = int(sqrt(1.0));
    i * (i = 3);
    sk_FragColor.x = float(i);
}
