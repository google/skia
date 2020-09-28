
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_hard_light;
    {
        vec4 _7_blend_overlay;
        {
            float _9_blend_overlay_component;
            vec2 _10_s = dst.xw;
            vec2 _11_d = src.xw;
            {
                _9_blend_overlay_component = 2.0 * _11_d.x <= _11_d.y ? (2.0 * _10_s.x) * _11_d.x : _10_s.y * _11_d.y - (2.0 * (_11_d.y - _11_d.x)) * (_10_s.y - _10_s.x);
            }
            float _12_blend_overlay_component;
            vec2 _13_s = dst.yw;
            vec2 _14_d = src.yw;
            {
                _12_blend_overlay_component = 2.0 * _14_d.x <= _14_d.y ? (2.0 * _13_s.x) * _14_d.x : _13_s.y * _14_d.y - (2.0 * (_14_d.y - _14_d.x)) * (_13_s.y - _13_s.x);
            }
            float _15_blend_overlay_component;
            vec2 _16_s = dst.zw;
            vec2 _17_d = src.zw;
            {
                _15_blend_overlay_component = 2.0 * _17_d.x <= _17_d.y ? (2.0 * _16_s.x) * _17_d.x : _16_s.y * _17_d.y - (2.0 * (_17_d.y - _17_d.x)) * (_16_s.y - _16_s.x);
            }
            vec4 _8_result = vec4(_9_blend_overlay_component, _12_blend_overlay_component, _15_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _8_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _7_blend_overlay = _8_result;
        }
        _0_blend_hard_light = _7_blend_overlay;

    }

    sk_FragColor = _0_blend_hard_light;

}
