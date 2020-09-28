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
    float4 _17_blend_hue;
    {
        float _18_alpha = _in.dst.w * _in.src.w;
        float3 _19_sda = _in.src.xyz * _in.dst.w;
        float3 _20_dsa = _in.dst.xyz * _in.src.w;
        float3 _21_blend_set_color_saturation;
        float3 _22_hueLumColor = _19_sda;
        {
            float _24_blend_color_saturation;
            {
                _24_blend_color_saturation = max(max(_20_dsa.x, _20_dsa.y), _20_dsa.z) - min(min(_20_dsa.x, _20_dsa.y), _20_dsa.z);
            }
            float _23_sat = _24_blend_color_saturation;

            if (_22_hueLumColor.x <= _22_hueLumColor.y) {
                if (_22_hueLumColor.y <= _22_hueLumColor.z) {
                    float3 _25_blend_set_color_saturation_helper;
                    {
                        _25_blend_set_color_saturation_helper = _22_hueLumColor.x < _22_hueLumColor.z ? float3(0.0, (_23_sat * (_22_hueLumColor.y - _22_hueLumColor.x)) / (_22_hueLumColor.z - _22_hueLumColor.x), _23_sat) : float3(0.0);
                    }
                    _22_hueLumColor.xyz = _25_blend_set_color_saturation_helper;

                } else if (_22_hueLumColor.x <= _22_hueLumColor.z) {
                    float3 _26_blend_set_color_saturation_helper;
                    float3 _27_minMidMax = _22_hueLumColor.xzy;
                    {
                        _26_blend_set_color_saturation_helper = _27_minMidMax.x < _27_minMidMax.z ? float3(0.0, (_23_sat * (_27_minMidMax.y - _27_minMidMax.x)) / (_27_minMidMax.z - _27_minMidMax.x), _23_sat) : float3(0.0);
                    }
                    _22_hueLumColor.xzy = _26_blend_set_color_saturation_helper;

                } else {
                    float3 _28_blend_set_color_saturation_helper;
                    float3 _29_minMidMax = _22_hueLumColor.zxy;
                    {
                        _28_blend_set_color_saturation_helper = _29_minMidMax.x < _29_minMidMax.z ? float3(0.0, (_23_sat * (_29_minMidMax.y - _29_minMidMax.x)) / (_29_minMidMax.z - _29_minMidMax.x), _23_sat) : float3(0.0);
                    }
                    _22_hueLumColor.zxy = _28_blend_set_color_saturation_helper;

                }
            } else if (_22_hueLumColor.x <= _22_hueLumColor.z) {
                float3 _30_blend_set_color_saturation_helper;
                float3 _31_minMidMax = _22_hueLumColor.yxz;
                {
                    _30_blend_set_color_saturation_helper = _31_minMidMax.x < _31_minMidMax.z ? float3(0.0, (_23_sat * (_31_minMidMax.y - _31_minMidMax.x)) / (_31_minMidMax.z - _31_minMidMax.x), _23_sat) : float3(0.0);
                }
                _22_hueLumColor.yxz = _30_blend_set_color_saturation_helper;

            } else if (_22_hueLumColor.y <= _22_hueLumColor.z) {
                float3 _32_blend_set_color_saturation_helper;
                float3 _33_minMidMax = _22_hueLumColor.yzx;
                {
                    _32_blend_set_color_saturation_helper = _33_minMidMax.x < _33_minMidMax.z ? float3(0.0, (_23_sat * (_33_minMidMax.y - _33_minMidMax.x)) / (_33_minMidMax.z - _33_minMidMax.x), _23_sat) : float3(0.0);
                }
                _22_hueLumColor.yzx = _32_blend_set_color_saturation_helper;

            } else {
                float3 _34_blend_set_color_saturation_helper;
                float3 _35_minMidMax = _22_hueLumColor.zyx;
                {
                    _34_blend_set_color_saturation_helper = _35_minMidMax.x < _35_minMidMax.z ? float3(0.0, (_23_sat * (_35_minMidMax.y - _35_minMidMax.x)) / (_35_minMidMax.z - _35_minMidMax.x), _23_sat) : float3(0.0);
                }
                _22_hueLumColor.zyx = _34_blend_set_color_saturation_helper;

            }
            _21_blend_set_color_saturation = _22_hueLumColor;
        }
        float3 _36_blend_set_color_luminance;
        {
            float _41_blend_color_luminance;
            {
                _41_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _20_dsa);
            }
            float _37_lum = _41_blend_color_luminance;

            float _42_blend_color_luminance;
            {
                _42_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _21_blend_set_color_saturation);
            }
            float3 _38_result = (_37_lum - _42_blend_color_luminance) + _21_blend_set_color_saturation;

            float _39_minComp = min(min(_38_result.x, _38_result.y), _38_result.z);
            float _40_maxComp = max(max(_38_result.x, _38_result.y), _38_result.z);
            if (_39_minComp < 0.0 && _37_lum != _39_minComp) {
                _38_result = _37_lum + ((_38_result - _37_lum) * _37_lum) / (_37_lum - _39_minComp);
            }
            _36_blend_set_color_luminance = _40_maxComp > _18_alpha && _40_maxComp != _37_lum ? _37_lum + ((_38_result - _37_lum) * (_18_alpha - _37_lum)) / (_40_maxComp - _37_lum) : _38_result;
        }
        _17_blend_hue = float4((((_36_blend_set_color_luminance + _in.dst.xyz) - _20_dsa) + _in.src.xyz) - _19_sda, (_in.src.w + _in.dst.w) - _18_alpha);


    }
    _out->sk_FragColor = _17_blend_hue;

    return *_out;
}
