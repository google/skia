
uniform vec4 src, dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    float _1_blend_overlay_component;
    vec2 _2_s = src.xw;
    vec2 _3_d = dst.xw;
    {
        _1_blend_overlay_component = 2.0 * _3_d.x <= _3_d.y ? (2.0 * _2_s.x) * _3_d.x : _2_s.y * _3_d.y - (2.0 * (_3_d.y - _3_d.x)) * (_2_s.y - _2_s.x);
    }
    float _4_blend_overlay_component;
    vec2 _5_s = src.yw;
    vec2 _6_d = dst.yw;
    {
        _4_blend_overlay_component = 2.0 * _6_d.x <= _6_d.y ? (2.0 * _5_s.x) * _6_d.x : _5_s.y * _6_d.y - (2.0 * (_6_d.y - _6_d.x)) * (_5_s.y - _5_s.x);
    }
    float _7_blend_overlay_component;
    vec2 _8_s = src.zw;
    vec2 _9_d = dst.zw;
    {
        _7_blend_overlay_component = 2.0 * _9_d.x <= _9_d.y ? (2.0 * _8_s.x) * _9_d.x : _8_s.y * _9_d.y - (2.0 * (_9_d.y - _9_d.x)) * (_8_s.y - _8_s.x);
    }
    vec4 result = vec4(_1_blend_overlay_component, _4_blend_overlay_component, _7_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
vec4 blend_hard_light(vec4 src, vec4 dst) {
    return blend_overlay(dst, src);
}
vec4 main() {
    vec4 _0_blend_hard_light;
    {
        _0_blend_hard_light = blend_overlay(dst, src);
    }

    return _0_blend_hard_light;

}
