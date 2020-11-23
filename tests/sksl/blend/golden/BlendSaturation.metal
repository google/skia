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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_saturation;
    {
        float _1_alpha = _in.dst.w * _in.src.w;
        float3 _2_sda = _in.src.xyz * _in.dst.w;
        float3 _3_dsa = _in.dst.xyz * _in.src.w;
        float3 _4_blend_set_color_saturation;
        float3 _5_hueLumColor = _3_dsa;
        {
            float _6_17_blend_color_saturation;
            {
                _6_17_blend_color_saturation = max(max(_2_sda.x, _2_sda.y), _2_sda.z) - min(min(_2_sda.x, _2_sda.y), _2_sda.z);
            }
            float _7_sat = _6_17_blend_color_saturation;

            if (_5_hueLumColor.x <= _5_hueLumColor.y) {
                if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                    float3 _8_18_blend_set_color_saturation_helper;
                    {
                        _8_18_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.z ? float3(0.0, (_7_sat * (_5_hueLumColor.y - _5_hueLumColor.x)) / (_5_hueLumColor.z - _5_hueLumColor.x), _7_sat) : float3(0.0);
                    }
                    _5_hueLumColor.xyz = _8_18_blend_set_color_saturation_helper;

                } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                    float3 _9_19_blend_set_color_saturation_helper;
                    {
                        _9_19_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.y ? float3(0.0, (_7_sat * (_5_hueLumColor.z - _5_hueLumColor.x)) / (_5_hueLumColor.y - _5_hueLumColor.x), _7_sat) : float3(0.0);
                    }
                    _5_hueLumColor.xzy = _9_19_blend_set_color_saturation_helper;

                } else {
                    float3 _10_20_blend_set_color_saturation_helper;
                    {
                        _10_20_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.y ? float3(0.0, (_7_sat * (_5_hueLumColor.x - _5_hueLumColor.z)) / (_5_hueLumColor.y - _5_hueLumColor.z), _7_sat) : float3(0.0);
                    }
                    _5_hueLumColor.zxy = _10_20_blend_set_color_saturation_helper;

                }
            } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                float3 _11_21_blend_set_color_saturation_helper;
                {
                    _11_21_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.z ? float3(0.0, (_7_sat * (_5_hueLumColor.x - _5_hueLumColor.y)) / (_5_hueLumColor.z - _5_hueLumColor.y), _7_sat) : float3(0.0);
                }
                _5_hueLumColor.yxz = _11_21_blend_set_color_saturation_helper;

            } else if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                float3 _12_22_blend_set_color_saturation_helper;
                {
                    _12_22_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.x ? float3(0.0, (_7_sat * (_5_hueLumColor.z - _5_hueLumColor.y)) / (_5_hueLumColor.x - _5_hueLumColor.y), _7_sat) : float3(0.0);
                }
                _5_hueLumColor.yzx = _12_22_blend_set_color_saturation_helper;

            } else {
                float3 _13_23_blend_set_color_saturation_helper;
                {
                    _13_23_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.x ? float3(0.0, (_7_sat * (_5_hueLumColor.y - _5_hueLumColor.z)) / (_5_hueLumColor.x - _5_hueLumColor.z), _7_sat) : float3(0.0);
                }
                _5_hueLumColor.zyx = _13_23_blend_set_color_saturation_helper;

            }
            _4_blend_set_color_saturation = _5_hueLumColor;
        }
        float3 _14_blend_set_color_luminance;
        {
            float _15_15_blend_color_luminance;
            {
                _15_15_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _16_lum = _15_15_blend_color_luminance;

            float _17_16_blend_color_luminance;
            {
                _17_16_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);
            }
            float3 _18_result = (_16_lum - _17_16_blend_color_luminance) + _4_blend_set_color_saturation;

            float _19_minComp = min(min(_18_result.x, _18_result.y), _18_result.z);
            float _20_maxComp = max(max(_18_result.x, _18_result.y), _18_result.z);
            if (_19_minComp < 0.0 && _16_lum != _19_minComp) {
                _18_result = _16_lum + ((_18_result - _16_lum) * _16_lum) / (_16_lum - _19_minComp);
            }
            _14_blend_set_color_luminance = _20_maxComp > _1_alpha && _20_maxComp != _16_lum ? _16_lum + ((_18_result - _16_lum) * (_1_alpha - _16_lum)) / (_20_maxComp - _16_lum) : _18_result;
        }
        _0_blend_saturation = float4((((_14_blend_set_color_luminance + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);


    }
    _out->sk_FragColor = _0_blend_saturation;

    return *_out;
}
