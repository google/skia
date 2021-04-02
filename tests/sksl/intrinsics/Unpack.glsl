
out vec4 sk_FragColor;
uniform uint a;
void main() {
    sk_FragColor.xy = vec2(unpackHalf2x16(a));
    sk_FragColor.xy = vec2(unpackUnorm2x16(a));
    sk_FragColor.xy = vec2(unpackSnorm2x16(a));
    sk_FragColor = vec4(unpackUnorm4x8(a));
    sk_FragColor = vec4(unpackSnorm4x8(a));
}
