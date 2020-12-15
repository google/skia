
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    sk_FragColor = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);

}
