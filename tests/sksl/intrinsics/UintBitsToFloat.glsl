
out vec4 sk_FragColor;
in uint a;
void main() {
    sk_FragColor.x = uintBitsToFloat(a);
}
