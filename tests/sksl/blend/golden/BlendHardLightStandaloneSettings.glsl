
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
vec4 blend_hard_light(vec4 src, vec4 dst) {
    return blend_overlay(dst, src);
}
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _1_blend_overlay;
        {
            float _3_blend_overlay_component;
            {
                _3_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _4_blend_overlay_component;
            {
                _4_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _5_blend_overlay_component;
            {
                _5_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _2_result = vec4(_3_blend_overlay_component, _4_blend_overlay_component, _5_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _2_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _1_blend_overlay = _2_result;
        }
        _0_blend_hard_light = _1_blend_overlay;

    }

    sk_FragColor = _0_blend_hard_light;

}
