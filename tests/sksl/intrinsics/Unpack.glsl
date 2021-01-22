
out vec4 sk_FragColor;
in uint a;
void main() {
    sk_FragColor.xy = unpackHalf2x16(a);
    sk_FragColor.xy = unpackUnorm2x16(a);
    sk_FragColor.xy = unpackSnorm2x16(a);
    sk_FragColor = unpackUnorm4x8(a);
    sk_FragColor = unpackSnorm4x8(a);
}
