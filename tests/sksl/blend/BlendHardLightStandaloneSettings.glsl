
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float blend_overlay_component_Qhh2h2(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
void main() {
    vec4 _0_result = vec4(blend_overlay_component_Qhh2h2(dst.xw, src.xw), blend_overlay_component_Qhh2h2(dst.yw, src.yw), blend_overlay_component_Qhh2h2(dst.zw, src.zw), dst.w + (1.0 - dst.w) * src.w);
    _0_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
    sk_FragColor = _0_result;
}
