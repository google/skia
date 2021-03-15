
out vec4 sk_FragColor;
float a;
void main() {
    sk_FragColor.x = log2(a);
}
