#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_multiply(vec4 src, vec4 dst) {
    return vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
}
void main() {
    vec4 _0_blend_multiply;
    {
        _0_blend_multiply = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _0_blend_multiply;

}
