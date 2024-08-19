cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _16_src : packoffset(c0);
    float4 _16_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float blend_color_saturation_Qhh3(float3 _22)
{
    return max(max(_22.x, _22.y), _22.z) - min(min(_22.x, _22.y), _22.z);
}

float4 blend_hslc_h4h2h4h4(float2 _45, float4 _46, float4 _47)
{
    float _55 = _47.w * _46.w;
    float alpha = _55;
    float3 _61 = _46.xyz * _47.w;
    float3 sda = _61;
    float3 _67 = _47.xyz * _46.w;
    float3 dsa = _67;
    float3 _72 = 0.0f.xxx;
    if (_45.x != 0.0f)
    {
        _72 = _67;
    }
    else
    {
        _72 = _61;
    }
    float3 l = _72;
    float3 _81 = 0.0f.xxx;
    if (_45.x != 0.0f)
    {
        _81 = _61;
    }
    else
    {
        _81 = _67;
    }
    float3 r = _81;
    if (_45.y != 0.0f)
    {
        float _92 = min(min(_72.x, _72.y), _72.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _92;
        float _98 = max(max(_72.x, _72.y), _72.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _98;
        float3 _101 = 0.0f.xxx;
        if (_98 > _92)
        {
            float3 _107 = _81;
            _101 = ((_72 - _92.xxx) * blend_color_saturation_Qhh3(_107)) * (1.0f / (_98 - _92));
        }
        else
        {
            _101 = 0.0f.xxx;
        }
        l = _101;
        r = _67;
    }
    float _117 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_4_lum = _117;
    float3 _129 = (_117 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_5_result = _129;
    float _133 = _129.x;
    float _134 = _129.y;
    float _135 = _129.z;
    float _131 = min(min(_133, _134), _135);
    float _RESERVED_IDENTIFIER_FIXUP_6_minComp = _131;
    float _137 = max(max(_133, _134), _135);
    float _RESERVED_IDENTIFIER_FIXUP_7_maxComp = _137;
    bool _143 = false;
    if (_131 < 0.0f)
    {
        _143 = _117 != _131;
    }
    else
    {
        _143 = false;
    }
    if (_143)
    {
        float3 _146 = _117.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _146 + ((_129 - _146) * (_117 / ((_117 - _131) + _kGuardedDivideEpsilon)));
    }
    bool _158 = false;
    if (_137 > _55)
    {
        _158 = _137 != _117;
    }
    else
    {
        _158 = false;
    }
    if (_158)
    {
        float3 _162 = _117.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _162 + (((_RESERVED_IDENTIFIER_FIXUP_5_result - _162) * (_55 - _117)) * (1.0f / ((_137 - _117) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_5_result + _47.xyz) - _67) + _46.xyz) - _61, (_46.w + _47.w) - _55);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _195 = 1.0f.xx;
    float4 _201 = _16_src;
    float4 _205 = _16_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_195, _201, _205);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
