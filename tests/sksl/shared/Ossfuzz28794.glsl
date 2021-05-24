
out vec4 sk_FragColor;
void main() {
    int i = 1;
    i * (i = 3);
    sk_FragColor.x = float(i);
}
