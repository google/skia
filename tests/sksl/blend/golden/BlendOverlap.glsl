#version 400
uniform vec4 src, dst;
float _blend_overlay_component(float sc, float sa, float dc, float da) {
    return 2.0 * dc <= da ? (2.0 * sc) * dc : sa * da - (2.0 * (da - dc)) * (sa - sc);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    float _0_blend_overlay_component;
    float _1_sc = src.x;
    float _2_sa = src.w;
    float _3_dc = dst.x;
    float _4_da = dst.w;
    {
        _0_blend_overlay_component = 2.0 * _3_dc <= _4_da ? (2.0 * _1_sc) * _3_dc : _2_sa * _4_da - (2.0 * (_4_da - _3_dc)) * (_2_sa - _1_sc);
    }
    float _5_blend_overlay_component;
    float _6_sc = src.y;
    float _7_sa = src.w;
    float _8_dc = dst.y;
    float _9_da = dst.w;
    {
        _5_blend_overlay_component = 2.0 * _8_dc <= _9_da ? (2.0 * _6_sc) * _8_dc : _7_sa * _9_da - (2.0 * (_9_da - _8_dc)) * (_7_sa - _6_sc);
    }
    float _10_blend_overlay_component;
    float _11_sc = src.z;
    float _12_sa = src.w;
    float _13_dc = dst.z;
    float _14_da = dst.w;
    {
        _10_blend_overlay_component = 2.0 * _13_dc <= _14_da ? (2.0 * _11_sc) * _13_dc : _12_sa * _14_da - (2.0 * (_14_da - _13_dc)) * (_12_sa - _11_sc);
    }
    vec4 result = vec4(_0_blend_overlay_component, _5_blend_overlay_component, _10_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
vec4 main() {
    return blend_overlay(src, dst);
}
