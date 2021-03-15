
out vec4 sk_FragColor;
int a;
void main() {
    sk_FragColor.x = intBitsToFloat(a);
}
