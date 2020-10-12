#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_xor(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src + (1.0 - src.w) * dst;
}
void main() {
    vec4 _0_blend_xor;
    {
        _0_blend_xor = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
    }

    sk_FragColor = _0_blend_xor;

}
