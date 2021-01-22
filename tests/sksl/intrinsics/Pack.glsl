
out vec4 sk_FragColor;
in vec2 a;
in vec4 b;
void main() {
    sk_FragColor.x = float(packHalf2x16(a));
    sk_FragColor.x = float(packUnorm2x16(a));
    sk_FragColor.x = float(packSnorm2x16(a));
    sk_FragColor.x = float(packUnorm4x8(b));
    sk_FragColor.x = float(packSnorm4x8(b));
}
