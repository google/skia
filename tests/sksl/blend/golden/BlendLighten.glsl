#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_lighten;
    {
        vec4 _3_blend_src_over;
        {
            _3_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _1_result = _3_blend_src_over;

        _1_result.xyz = max(_1_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _0_blend_lighten = _1_result;
    }

    sk_FragColor = _0_blend_lighten;

}
