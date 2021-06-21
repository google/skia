
out vec4 sk_FragColor;
uniform uint a;
void main() {
    sk_FragColor.x = uintBitsToFloat(a);
}
