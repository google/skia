#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst);
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst) {
    return dst * src.w;
}
void main() {
    sk_FragColor = blend_dst_in_h4h4h4(src, dst);
}
