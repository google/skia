cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _13_src : packoffset(c0);
    float4 _13_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float3 _blend_set_color_saturation_helper_h3h3h(float3 _113, float _114)
{
    if (_113.x < _113.z)
    {
        return float3(0.0f, (_114 * (_113.y - _113.x)) / (_113.z - _113.x), _114);
    }
    else
    {
        return 0.0f.xxx;
    }
}

float3 _blend_set_color_saturation_h3h3h3(float3 _141, float3 _142)
{
    float sat = max(max(_142.x, _142.y), _142.z) - min(min(_142.x, _142.y), _142.z);
    if (_141.x <= _141.y)
    {
        if (_141.y <= _141.z)
        {
            float3 _179 = _141;
            float _181 = sat;
            return _blend_set_color_saturation_helper_h3h3h(_179, _181);
        }
        else
        {
            if (_141.x <= _141.z)
            {
                float3 _193 = _141.xzy;
                float _195 = sat;
                return _blend_set_color_saturation_helper_h3h3h(_193, _195).xzy;
            }
            else
            {
                float3 _200 = _141.zxy;
                float _202 = sat;
                return _blend_set_color_saturation_helper_h3h3h(_200, _202).yzx;
            }
        }
    }
    else
    {
        if (_141.x <= _141.z)
        {
            float3 _215 = _141.yxz;
            float _217 = sat;
            return _blend_set_color_saturation_helper_h3h3h(_215, _217).yxz;
        }
        else
        {
            if (_141.y <= _141.z)
            {
                float3 _230 = _141.yzx;
                float _232 = sat;
                return _blend_set_color_saturation_helper_h3h3h(_230, _232).zxy;
            }
            else
            {
                float3 _237 = _141.zyx;
                float _239 = sat;
                return _blend_set_color_saturation_helper_h3h3h(_237, _239).zyx;
            }
        }
    }
}

float3 _blend_set_color_luminance_h3h3hh3(float3 _20, float _21, float3 _22)
{
    float lum = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _22);
    float3 result = (lum - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _20)).xxx + _20;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    bool _66 = false;
    if (minComp < 0.0f)
    {
        _66 = lum != minComp;
    }
    else
    {
        _66 = false;
    }
    if (_66)
    {
        result = lum.xxx + ((result - lum.xxx) * (lum / (lum - minComp)));
    }
    bool _90 = false;
    if (maxComp > _21)
    {
        _90 = maxComp != lum;
    }
    else
    {
        _90 = false;
    }
    if (_90)
    {
        return lum.xxx + (((result - lum.xxx) * (_21 - lum)) * (1.0f / (maxComp - lum)));
    }
    else
    {
        return result;
    }
}

void frag_main()
{
    float _RESERVED_IDENTIFIER_FIXUP_0_alpha = _13_dst.w * _13_src.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_sda = _13_src.xyz * _13_dst.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_2_dsa = _13_dst.xyz * _13_src.w;
    float3 _274 = _RESERVED_IDENTIFIER_FIXUP_1_sda;
    float3 _276 = _RESERVED_IDENTIFIER_FIXUP_2_dsa;
    float3 _278 = _blend_set_color_saturation_h3h3h3(_274, _276);
    float _280 = _RESERVED_IDENTIFIER_FIXUP_0_alpha;
    float3 _282 = _RESERVED_IDENTIFIER_FIXUP_2_dsa;
    sk_FragColor = float4((((_blend_set_color_luminance_h3h3hh3(_278, _280, _282) + _13_dst.xyz) - _RESERVED_IDENTIFIER_FIXUP_2_dsa) + _13_src.xyz) - _RESERVED_IDENTIFIER_FIXUP_1_sda, (_13_src.w + _13_dst.w) - _RESERVED_IDENTIFIER_FIXUP_0_alpha);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
