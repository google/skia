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
    float4 _3_blend_overlay;
    {
        float _5_blend_overlay_component;
        float2 _6_s = _in.src.xw;
        float2 _7_d = _in.dst.xw;
        {
            _5_blend_overlay_component = 2.0 * _7_d.x <= _7_d.y ? (2.0 * _6_s.x) * _7_d.x : _6_s.y * _7_d.y - (2.0 * (_7_d.y - _7_d.x)) * (_6_s.y - _6_s.x);
        }
        float _8_blend_overlay_component;
        float2 _9_s = _in.src.yw;
        float2 _10_d = _in.dst.yw;
        {
            _8_blend_overlay_component = 2.0 * _10_d.x <= _10_d.y ? (2.0 * _9_s.x) * _10_d.x : _9_s.y * _10_d.y - (2.0 * (_10_d.y - _10_d.x)) * (_9_s.y - _9_s.x);
        }
        float _11_blend_overlay_component;
        float2 _12_s = _in.src.zw;
        float2 _13_d = _in.dst.zw;
        {
            _11_blend_overlay_component = 2.0 * _13_d.x <= _13_d.y ? (2.0 * _12_s.x) * _13_d.x : _12_s.y * _13_d.y - (2.0 * (_13_d.y - _13_d.x)) * (_12_s.y - _12_s.x);
        }
        float4 _4_result = float4(_5_blend_overlay_component, _8_blend_overlay_component, _11_blend_overlay_component, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);



        _4_result.xyz = _4_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
        _3_blend_overlay = _4_result;
    }
    _out->sk_FragColor = _3_blend_overlay;

    return *_out;
}
