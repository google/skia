#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_out(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src;
}
void main() {
    vec4 _0_blend_src_out;
    {
        _0_blend_src_out = (1.0 - dst.w) * src;
    }

    sk_FragColor = _0_blend_src_out;

}
