
uniform vec4 src, dst;
float _blend_overlay_component(float sc, float sa, float dc, float da) {
    if (2.0 * dc <= da) {
        return (2.0 * sc) * dc;
    }
    return sa * da - (2.0 * (da - dc)) * (sa - sc);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.x, src.w, dst.x, dst.w), _blend_overlay_component(src.y, src.w, dst.y, dst.w), _blend_overlay_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
vec4 main() {
    return blend_overlay(src, dst);
}
