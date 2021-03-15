
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    sk_FragColor = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
}
