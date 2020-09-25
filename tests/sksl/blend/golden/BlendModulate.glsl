#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
vec4 blend_modulate(vec4 src, vec4 dst) {
    return src * dst;
}
void main() {
    vec4 _0_blend_modulate;
    {
        _0_blend_modulate = src * dst;
    }

    sk_FragColor = _0_blend_modulate;

}
