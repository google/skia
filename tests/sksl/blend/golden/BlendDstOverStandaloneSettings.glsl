
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_dst_over(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src + dst;
}
void main() {
    vec4 _0_blend_dst_over;
    {
        _0_blend_dst_over = (1.0 - dst.w) * src + dst;
    }

    sk_FragColor = _0_blend_dst_over;

}
