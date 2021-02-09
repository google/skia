
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src == vec4(0.0) ? vec4(0.0) : src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    return dst == vec4(0.0) ? vec4(0.0) : dst * src.w;

}
void main() {
    sk_FragColor = blend_src_in(src, dst);
    sk_FragColor = blend_dst_in(src, dst);
}
