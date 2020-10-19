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
    float4 result = float4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float4 blend_hard_light(float4 src, float4 dst) {
    return blend_overlay(dst, src);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_hard_light;
    {
        float4 _1_blend_overlay;
        {
            float _3_blend_overlay_component;
            {
                _3_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _4_blend_overlay_component;
            {
                _4_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _5_blend_overlay_component;
            {
                _5_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _2_result = float4(_3_blend_overlay_component, _4_blend_overlay_component, _5_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _2_result.xyz = _2_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _1_blend_overlay = _2_result;
        }
        _0_blend_hard_light = _1_blend_overlay;

    }

    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
