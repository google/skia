
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst);
vec4 blend_darken_h4h4h4h(vec4 src, vec4 dst, float mode);
vec4 blend_darken_h4h4h4(vec4 src, vec4 dst);
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
vec4 blend_darken_h4h4h4h(vec4 src, vec4 dst, float mode) {
    vec4 a = blend_src_over_h4h4h4(src, dst);
    vec3 b = (1.0 - dst.w) * src.xyz + dst.xyz;
    a.xyz = mode * min(a.xyz * mode, b * mode);
    return a;
}
vec4 blend_darken_h4h4h4(vec4 src, vec4 dst) {
    return blend_darken_h4h4h4h(src, dst, 1.0);
}
void main() {
    sk_FragColor = blend_darken_h4h4h4(src, dst);
}
