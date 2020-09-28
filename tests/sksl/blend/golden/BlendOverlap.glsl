#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _3_blend_overlay;
    {
        float _5_blend_overlay_component;
        vec2 _6_s = src.xw;
        vec2 _7_d = dst.xw;
        {
            _5_blend_overlay_component = 2.0 * _7_d.x <= _7_d.y ? (2.0 * _6_s.x) * _7_d.x : _6_s.y * _7_d.y - (2.0 * (_7_d.y - _7_d.x)) * (_6_s.y - _6_s.x);
        }
        float _8_blend_overlay_component;
        vec2 _9_s = src.yw;
        vec2 _10_d = dst.yw;
        {
            _8_blend_overlay_component = 2.0 * _10_d.x <= _10_d.y ? (2.0 * _9_s.x) * _10_d.x : _9_s.y * _10_d.y - (2.0 * (_10_d.y - _10_d.x)) * (_9_s.y - _9_s.x);
        }
        float _11_blend_overlay_component;
        vec2 _12_s = src.zw;
        vec2 _13_d = dst.zw;
        {
            _11_blend_overlay_component = 2.0 * _13_d.x <= _13_d.y ? (2.0 * _12_s.x) * _13_d.x : _12_s.y * _13_d.y - (2.0 * (_13_d.y - _13_d.x)) * (_12_s.y - _12_s.x);
        }
        vec4 _4_result = vec4(_5_blend_overlay_component, _8_blend_overlay_component, _11_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _4_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _3_blend_overlay = _4_result;
    }
    sk_FragColor = _3_blend_overlay;

}
