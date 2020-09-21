#version 400
uniform vec4 src, dst;
vec4 blend_plus(vec4 src, vec4 dst) {
    return min(src + dst, 1.0);
}
vec4 main() {
    vec4 _0_blend_plus;
    {
        _0_blend_plus = min(src + dst, 1.0);
    }

    return _0_blend_plus;

}
