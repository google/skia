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
    float4 _12_blend_saturation;
    {
        float _13_alpha = _in.dst.w * _in.src.w;
        float3 _14_sda = _in.src.xyz * _in.dst.w;
        float3 _15_dsa = _in.dst.xyz * _in.src.w;
        float3 _16_blend_set_color_saturation;
        float3 _17_hueLumColor = _15_dsa;
        {
            float _19_blend_color_saturation;
            {
                _19_blend_color_saturation = max(max(_14_sda.x, _14_sda.y), _14_sda.z) - min(min(_14_sda.x, _14_sda.y), _14_sda.z);
            }
            float _18_sat = _19_blend_color_saturation;

            if (_17_hueLumColor.x <= _17_hueLumColor.y) {
                if (_17_hueLumColor.y <= _17_hueLumColor.z) {
                    float3 _20_blend_set_color_saturation_helper;
                    {
                        _20_blend_set_color_saturation_helper = _17_hueLumColor.x < _17_hueLumColor.z ? float3(0.0, (_18_sat * (_17_hueLumColor.y - _17_hueLumColor.x)) / (_17_hueLumColor.z - _17_hueLumColor.x), _18_sat) : float3(0.0);
                    }
                    _17_hueLumColor.xyz = _20_blend_set_color_saturation_helper;

                } else if (_17_hueLumColor.x <= _17_hueLumColor.z) {
                    float3 _21_blend_set_color_saturation_helper;
                    {
                        _21_blend_set_color_saturation_helper = _17_hueLumColor.x < _17_hueLumColor.y ? float3(0.0, (_18_sat * (_17_hueLumColor.z - _17_hueLumColor.x)) / (_17_hueLumColor.y - _17_hueLumColor.x), _18_sat) : float3(0.0);
                    }
                    _17_hueLumColor.xzy = _21_blend_set_color_saturation_helper;

                } else {
                    float3 _22_blend_set_color_saturation_helper;
                    {
                        _22_blend_set_color_saturation_helper = _17_hueLumColor.z < _17_hueLumColor.y ? float3(0.0, (_18_sat * (_17_hueLumColor.x - _17_hueLumColor.z)) / (_17_hueLumColor.y - _17_hueLumColor.z), _18_sat) : float3(0.0);
                    }
                    _17_hueLumColor.zxy = _22_blend_set_color_saturation_helper;

                }
            } else if (_17_hueLumColor.x <= _17_hueLumColor.z) {
                float3 _23_blend_set_color_saturation_helper;
                {
                    _23_blend_set_color_saturation_helper = _17_hueLumColor.y < _17_hueLumColor.z ? float3(0.0, (_18_sat * (_17_hueLumColor.x - _17_hueLumColor.y)) / (_17_hueLumColor.z - _17_hueLumColor.y), _18_sat) : float3(0.0);
                }
                _17_hueLumColor.yxz = _23_blend_set_color_saturation_helper;

            } else if (_17_hueLumColor.y <= _17_hueLumColor.z) {
                float3 _24_blend_set_color_saturation_helper;
                {
                    _24_blend_set_color_saturation_helper = _17_hueLumColor.y < _17_hueLumColor.x ? float3(0.0, (_18_sat * (_17_hueLumColor.z - _17_hueLumColor.y)) / (_17_hueLumColor.x - _17_hueLumColor.y), _18_sat) : float3(0.0);
                }
                _17_hueLumColor.yzx = _24_blend_set_color_saturation_helper;

            } else {
                float3 _25_blend_set_color_saturation_helper;
                {
                    _25_blend_set_color_saturation_helper = _17_hueLumColor.z < _17_hueLumColor.x ? float3(0.0, (_18_sat * (_17_hueLumColor.y - _17_hueLumColor.z)) / (_17_hueLumColor.x - _17_hueLumColor.z), _18_sat) : float3(0.0);
                }
                _17_hueLumColor.zyx = _25_blend_set_color_saturation_helper;

            }
            _16_blend_set_color_saturation = _17_hueLumColor;
        }
        float3 _26_blend_set_color_luminance;
        {
            float _31_blend_color_luminance;
            {
                _31_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _15_dsa);
            }
            float _27_lum = _31_blend_color_luminance;

            float _32_blend_color_luminance;
            {
                _32_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _16_blend_set_color_saturation);
            }
            float3 _28_result = (_27_lum - _32_blend_color_luminance) + _16_blend_set_color_saturation;

            float _29_minComp = min(min(_28_result.x, _28_result.y), _28_result.z);
            float _30_maxComp = max(max(_28_result.x, _28_result.y), _28_result.z);
            if (_29_minComp < 0.0 && _27_lum != _29_minComp) {
                _28_result = _27_lum + ((_28_result - _27_lum) * _27_lum) / (_27_lum - _29_minComp);
            }
            _26_blend_set_color_luminance = _30_maxComp > _13_alpha && _30_maxComp != _27_lum ? _27_lum + ((_28_result - _27_lum) * (_13_alpha - _27_lum)) / (_30_maxComp - _27_lum) : _28_result;
        }
        _12_blend_saturation = float4((((_26_blend_set_color_luminance + _in.dst.xyz) - _15_dsa) + _in.src.xyz) - _14_sda, (_in.src.w + _in.dst.w) - _13_alpha);


    }
    _out->sk_FragColor = _12_blend_saturation;

    return *_out;
}
