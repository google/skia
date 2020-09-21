#version 400
uniform vec4 src, dst;
vec4 blend_modulate(vec4 src, vec4 dst) {
    return src * dst;
}
vec4 main() {
    vec4 _0_blend_modulate;
    {
        _0_blend_modulate = src * dst;
    }

    return _0_blend_modulate;

}
