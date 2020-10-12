
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_dst_out(vec4 src, vec4 dst) {
    return (1.0 - src.w) * dst;
}
void main() {
    vec4 _0_blend_dst_out;
    {
        _0_blend_dst_out = (1.0 - src.w) * dst;
    }

    sk_FragColor = _0_blend_dst_out;

}
