
out vec4 sk_FragColor;
int a;
uint b;
void main() {
    sk_FragColor.x = float(bitCount(a));
    sk_FragColor.y = float(bitCount(b));
}
