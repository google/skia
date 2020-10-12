
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_over(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
vec4 blend_darken(vec4 src, vec4 dst) {
    vec4 _2_blend_src_over;
    {
        _2_blend_src_over = src + (1.0 - src.w) * dst;
    }
    vec4 result = _2_blend_src_over;

    result.xyz = min(result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    return result;
}
void main() {
    vec4 _0_blend_darken;
    {
        vec4 _3_blend_src_over;
        {
            _3_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _1_result = _3_blend_src_over;

        _1_result.xyz = min(_1_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _0_blend_darken = _1_result;
    }

    sk_FragColor = _0_blend_darken;

}
