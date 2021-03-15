#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_lighten(vec4 src, vec4 dst) {
    vec4 result = src + (1.0 - src.w) * dst;
    result.xyz = max(result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    return result;
}
void main() {
    sk_FragColor = blend_lighten(src, dst);
}
