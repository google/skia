
out vec4 sk_FragColor;
in float a;
in vec4 b;
void main() {
    sk_FragColor.x = float(floatBitsToInt(a));
    sk_FragColor = vec4(floatBitsToInt(b));
}
