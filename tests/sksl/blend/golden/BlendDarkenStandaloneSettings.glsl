
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _1_result = src + (1.0 - src.w) * dst;

    _1_result.xyz = min(_1_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    sk_FragColor = _1_result;

}
