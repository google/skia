
out vec4 sk_FragColor;
uniform int a;
void main() {
    sk_FragColor.x = intBitsToFloat(a);
}
