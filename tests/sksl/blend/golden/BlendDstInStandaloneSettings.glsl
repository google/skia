
out vec4 sk_FragColor;
in vec4 src, dst;
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
