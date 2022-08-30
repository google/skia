#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_exclusion_h4h4h4(vec4 src, vec4 dst);
vec4 blend_exclusion_h4h4h4(vec4 src, vec4 dst) {
    return vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
}
void main() {
    sk_FragColor = blend_exclusion_h4h4h4(src, dst);
}
