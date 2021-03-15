#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_result = src + (1.0 - src.w) * dst;
    _0_result.xyz = min(_0_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    sk_FragColor = _0_result;
}
