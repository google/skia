#version 400
uniform vec4 src, dst;
vec4 blend_src(vec4 src, vec4 dst) {
    return src;
}
vec4 main() {
    vec4 _0_blend_src;
    {
        _0_blend_src = src;
    }

    return _0_blend_src;

}
