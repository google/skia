
out vec4 sk_FragColor;
in int a;
void main() {
    sk_FragColor.x = intBitsToFloat(a);
}
