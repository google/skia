
uniform vec4 src, dst;
vec4 blend_screen(vec4 src, vec4 dst) {
    return src + (1.0 - src) * dst;
}
vec4 main() {
    vec4 _0_blend_screen;
    {
        _0_blend_screen = src + (1.0 - src) * dst;
    }

    return _0_blend_screen;

}
