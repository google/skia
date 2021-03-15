
out vec4 sk_FragColor;
float a;
float b;
void main() {
    sk_FragColor.x = step(a, b);
}
