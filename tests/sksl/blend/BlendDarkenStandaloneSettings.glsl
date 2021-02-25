
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_darken;
    vec4 _1_blend_src_over;
    vec4 _2_result = src + (1.0 - src.w) * dst;

    _2_result.xyz = min(_2_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    sk_FragColor = _2_result;

}
