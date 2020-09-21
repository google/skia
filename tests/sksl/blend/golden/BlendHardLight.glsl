#version 400
uniform vec4 src, dst;
float _blend_overlay_component(float sc, float sa, float dc, float da) {
    return 2.0 * dc <= da ? (2.0 * sc) * dc : sa * da - (2.0 * (da - dc)) * (sa - sc);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    float _1_blend_overlay_component;
    float _2_sc = src.x;
    float _3_sa = src.w;
    float _4_dc = dst.x;
    float _5_da = dst.w;
    {
        _1_blend_overlay_component = 2.0 * _4_dc <= _5_da ? (2.0 * _2_sc) * _4_dc : _3_sa * _5_da - (2.0 * (_5_da - _4_dc)) * (_3_sa - _2_sc);
    }
    float _6_blend_overlay_component;
    float _7_sc = src.y;
    float _8_sa = src.w;
    float _9_dc = dst.y;
    float _10_da = dst.w;
    {
        _6_blend_overlay_component = 2.0 * _9_dc <= _10_da ? (2.0 * _7_sc) * _9_dc : _8_sa * _10_da - (2.0 * (_10_da - _9_dc)) * (_8_sa - _7_sc);
    }
    float _11_blend_overlay_component;
    float _12_sc = src.z;
    float _13_sa = src.w;
    float _14_dc = dst.z;
    float _15_da = dst.w;
    {
        _11_blend_overlay_component = 2.0 * _14_dc <= _15_da ? (2.0 * _12_sc) * _14_dc : _13_sa * _15_da - (2.0 * (_15_da - _14_dc)) * (_13_sa - _12_sc);
    }
    vec4 result = vec4(_1_blend_overlay_component, _6_blend_overlay_component, _11_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



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
