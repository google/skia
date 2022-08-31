
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    vec4 _0_a = src + (1.0 - src.w) * dst;
    vec3 _1_b = (1.0 - dst.w) * src.xyz + dst.xyz;
    _0_a.xyz = min(_0_a.xyz, _1_b);
    sk_FragColor = _0_a;
}
