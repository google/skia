#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_dst_in;
    vec4 _1_blend_src_in;
    _1_blend_src_in = dst * src.w;

    _0_blend_dst_in = _1_blend_src_in;


    sk_FragColor = _0_blend_dst_in;

}
