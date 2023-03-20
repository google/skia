
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float blend_overlay_component_Qhh2h2(vec2 s, vec2 d);
float blend_overlay_component_Qhh2h2(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
void main() {
    vec4 _0_result = vec4(blend_overlay_component_Qhh2h2(src.xw, dst.xw), blend_overlay_component_Qhh2h2(src.yw, dst.yw), blend_overlay_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    _0_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    sk_FragColor = _0_result;
}
