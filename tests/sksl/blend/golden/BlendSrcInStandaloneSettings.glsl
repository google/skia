
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
void main() {
    vec4 _0_blend_src_in;
    {
        _0_blend_src_in = src * dst.w;
    }

    sk_FragColor = _0_blend_src_in;

}
