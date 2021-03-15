
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_darken(vec4 src, vec4 dst) {
    vec4 result = src + (1.0 - src.w) * dst;
    result.xyz = min(result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    return result;
}
void main() {
    sk_FragColor = blend_darken(src, dst);
}
