#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
void main() {
    vec4 _0_result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    _0_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    sk_FragColor = _0_result;
}
