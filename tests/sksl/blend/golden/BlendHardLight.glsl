#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _1_8_blend_overlay;
        {
            float _2_9_1_blend_overlay_component;
            {
                _2_9_1_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _3_76_blend_overlay_component;
            {
                _3_76_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _4_80_blend_overlay_component;
            {
                _4_80_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _5_10_result = vec4(_2_9_1_blend_overlay_component, _3_76_blend_overlay_component, _4_80_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _5_10_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _1_8_blend_overlay = _5_10_result;
        }
        _0_blend_hard_light = _1_8_blend_overlay;

    }
    sk_FragColor = _0_blend_hard_light;

}
