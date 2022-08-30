
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_multiply_h4h4h4(vec4 src, vec4 dst);
vec4 blend_multiply_h4h4h4(vec4 src, vec4 dst) {
    return vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
}
void main() {
    sk_FragColor = blend_multiply_h4h4h4(src, dst);
}
