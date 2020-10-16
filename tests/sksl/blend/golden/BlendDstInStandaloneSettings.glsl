
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    return blend_src_in(dst, src);
}
void main() {
    sk_FragColor = blend_dst_in(src, dst);
}
