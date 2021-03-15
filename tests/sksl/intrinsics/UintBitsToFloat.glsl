
out vec4 sk_FragColor;
uint a;
void main() {
    sk_FragColor.x = uintBitsToFloat(a);
}
