#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 srcdst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_hard_light;
    {
        float4 _7_blend_overlay;
        {
            float _9_blend_overlay_component;
            float2 _10_s = _in.dst.xw;
            float2 _11_d = _in.src.xw;
            {
                _9_blend_overlay_component = 2.0 * _11_d.x <= _11_d.y ? (2.0 * _10_s.x) * _11_d.x : _10_s.y * _11_d.y - (2.0 * (_11_d.y - _11_d.x)) * (_10_s.y - _10_s.x);
            }
            float _12_blend_overlay_component;
            float2 _13_s = _in.dst.yw;
            float2 _14_d = _in.src.yw;
            {
                _12_blend_overlay_component = 2.0 * _14_d.x <= _14_d.y ? (2.0 * _13_s.x) * _14_d.x : _13_s.y * _14_d.y - (2.0 * (_14_d.y - _14_d.x)) * (_13_s.y - _13_s.x);
            }
            float _15_blend_overlay_component;
            float2 _16_s = _in.dst.zw;
            float2 _17_d = _in.src.zw;
            {
                _15_blend_overlay_component = 2.0 * _17_d.x <= _17_d.y ? (2.0 * _16_s.x) * _17_d.x : _16_s.y * _17_d.y - (2.0 * (_17_d.y - _17_d.x)) * (_16_s.y - _16_s.x);
            }
            float4 _8_result = float4(_9_blend_overlay_component, _12_blend_overlay_component, _15_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _8_result.xyz = _8_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _7_blend_overlay = _8_result;
        }
        _0_blend_hard_light = _7_blend_overlay;

    }

    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
