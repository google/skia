
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
void main() {
    sk_FragColor = blend_overlay(dst, src);
}
