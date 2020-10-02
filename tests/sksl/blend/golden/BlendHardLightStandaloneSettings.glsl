
out vec4 sk_FragColor;
in vec4 src, dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _3_blend_overlay;
        {
            vec4 _4_result = vec4(_blend_overlay_component(dst.xw, src.xw), _blend_overlay_component(dst.yw, src.yw), _blend_overlay_component(dst.zw, src.zw), dst.w + (1.0 - dst.w) * src.w);
            _4_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _3_blend_overlay = _4_result;
        }
        _0_blend_hard_light = _3_blend_overlay;

    }

    sk_FragColor = _0_blend_hard_light;

}
