
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    vec4 _0_result = src + (1.0 - src.w) * dst;
    _0_result.xyz = max(_0_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    sk_FragColor = _0_result;
}
