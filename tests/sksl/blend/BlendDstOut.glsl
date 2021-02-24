#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_dst_out;
    sk_FragColor = (1.0 - src.w) * dst;

}
