#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst);
vec4 blend_lighten_h4h4h4(vec4 src, vec4 dst);
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
vec4 blend_lighten_h4h4h4(vec4 src, vec4 dst) {
    vec4 result = blend_src_over_h4h4h4(src, dst);
    result.xyz = max(result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    return result;
}
void main() {
    sk_FragColor = blend_lighten_h4h4h4(src, dst);
}
