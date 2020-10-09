
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _1_blend_lighten;
    {
        vec4 _3_blend_src_over;
        {
            _3_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _2_result = _3_blend_src_over;

        _2_result.xyz = max(_2_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _1_blend_lighten = _2_result;
    }
    sk_FragColor = _1_blend_lighten;

}
