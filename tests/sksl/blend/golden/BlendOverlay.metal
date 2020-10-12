#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


float _blend_overlay_component(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
float4 blend_overlay(float4 src, float4 dst) {
    float _2_blend_overlay_component;
    {
        _2_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
    }
    float _4_blend_overlay_component;
    {
        _4_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
    }
    float _6_blend_overlay_component;
    {
        _6_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
    }
    float4 result = float4(_2_blend_overlay_component, _4_blend_overlay_component, _6_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_overlay;
    {
        float _3_blend_overlay_component;
        {
            _3_blend_overlay_component = 2.0 * _in.dst.x <= _in.dst.w ? (2.0 * _in.src.x) * _in.dst.x : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.x)) * (_in.src.w - _in.src.x);
        }
        float _5_blend_overlay_component;
        {
            _5_blend_overlay_component = 2.0 * _in.dst.y <= _in.dst.w ? (2.0 * _in.src.y) * _in.dst.y : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.y)) * (_in.src.w - _in.src.y);
        }
        float _7_blend_overlay_component;
        {
            _7_blend_overlay_component = 2.0 * _in.dst.z <= _in.dst.w ? (2.0 * _in.src.z) * _in.dst.z : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.z)) * (_in.src.w - _in.src.z);
        }
        float4 _1_result = float4(_3_blend_overlay_component, _5_blend_overlay_component, _7_blend_overlay_component, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);



        _1_result.xyz = _1_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
        _0_blend_overlay = _1_result;
    }

    _out->sk_FragColor = _0_blend_overlay;

    return *_out;
}
