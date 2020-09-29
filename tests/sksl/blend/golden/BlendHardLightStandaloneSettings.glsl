
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _3_blend_overlay;
        {
            float _5_blend_overlay_component;
            {
                _5_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _6_blend_overlay_component;
            {
                _6_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _7_blend_overlay_component;
            {
                _7_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _4_result = vec4(_5_blend_overlay_component, _6_blend_overlay_component, _7_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _4_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _3_blend_overlay = _4_result;
        }
        _0_blend_hard_light = _3_blend_overlay;

    }

    sk_FragColor = _0_blend_hard_light;

}
