cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_src : packoffset(c0);
    float4 _11_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float3 _blend_set_color_luminance_h3h3hh3(float3 _18, float _19, float3 _20)
{
    float lum = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _20);
    float3 result = (lum - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), _18)).xxx + _18;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    bool _64 = false;
    if (minComp < 0.0f)
    {
        _64 = lum != minComp;
    }
    else
    {
        _64 = false;
    }
    if (_64)
    {
        result = lum.xxx + ((result - lum.xxx) * (lum / (lum - minComp)));
    }
    bool _88 = false;
    if (maxComp > _19)
    {
        _88 = maxComp != lum;
    }
    else
    {
        _88 = false;
    }
    if (_88)
    {
        return lum.xxx + (((result - lum.xxx) * (_19 - lum)) * (1.0f / (maxComp - lum)));
    }
    else
    {
        return result;
    }
}

void frag_main()
{
    float _RESERVED_IDENTIFIER_FIXUP_0_alpha = _11_dst.w * _11_src.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_1_sda = _11_src.xyz * _11_dst.w;
    float3 _RESERVED_IDENTIFIER_FIXUP_2_dsa = _11_dst.xyz * _11_src.w;
    float3 _142 = _RESERVED_IDENTIFIER_FIXUP_2_dsa;
    float _144 = _RESERVED_IDENTIFIER_FIXUP_0_alpha;
    float3 _146 = _RESERVED_IDENTIFIER_FIXUP_1_sda;
    sk_FragColor = float4((((_blend_set_color_luminance_h3h3hh3(_142, _144, _146) + _11_dst.xyz) - _RESERVED_IDENTIFIER_FIXUP_2_dsa) + _11_src.xyz) - _RESERVED_IDENTIFIER_FIXUP_1_sda, (_11_src.w + _11_dst.w) - _RESERVED_IDENTIFIER_FIXUP_0_alpha);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
