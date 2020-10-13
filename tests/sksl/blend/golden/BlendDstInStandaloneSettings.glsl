
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    vec4 _1_blend_src_in;
    {
        _1_blend_src_in = dst * src.w;
    }
    return _1_blend_src_in;

}
void main() {
    vec4 _0_blend_dst_in;
    {
        vec4 _2_blend_src_in;
        {
            _2_blend_src_in = dst * src.w;
        }
        _0_blend_dst_in = _2_blend_src_in;

    }

    sk_FragColor = _0_blend_dst_in;

}
