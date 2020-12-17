
out vec4 sk_FragColor;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
in vec4 src;
in vec4 dst;
void main() {
    vec4 _1_result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    _1_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    sk_FragColor = _1_result;

}
