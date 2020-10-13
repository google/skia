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
    float _1_blend_overlay_component;
    {
        _1_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
    }
    float _6_blend_overlay_component;
    {
        _6_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
    }
    float _9_blend_overlay_component;
    {
        _9_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
    }
    float4 result = float4(_1_blend_overlay_component, _6_blend_overlay_component, _9_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float4 blend_hard_light(float4 src, float4 dst) {
    float4 _2_blend_overlay;
    {
        float _7_blend_overlay_component;
        {
            _7_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
        }
        float _10_blend_overlay_component;
        {
            _10_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
        }
        float _12_blend_overlay_component;
        {
            _12_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
        }
        float4 _3_result = float4(_7_blend_overlay_component, _10_blend_overlay_component, _12_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



        _3_result.xyz = _3_result.xyz + src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
        _2_blend_overlay = _3_result;
    }
    return _2_blend_overlay;

}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_hard_light;
    {
        float4 _4_blend_overlay;
        {
            float _8_blend_overlay_component;
            {
                _8_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _11_blend_overlay_component;
            {
                _11_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _13_blend_overlay_component;
            {
                _13_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _5_result = float4(_8_blend_overlay_component, _11_blend_overlay_component, _13_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _5_result.xyz = _5_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _4_blend_overlay = _5_result;
        }
        _0_blend_hard_light = _4_blend_overlay;

    }

    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
