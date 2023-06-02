cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _18_src : packoffset(c0);
    float4 _18_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float blend_color_saturation_Qhh3(float3 _24)
{
    return max(max(_24.x, _24.y), _24.z) - min(min(_24.x, _24.y), _24.z);
}

float4 blend_hslc_h4h2h4h4(float2 _47, float4 _48, float4 _49)
{
    float _57 = _49.w * _48.w;
    float alpha = _57;
    float3 _63 = _48.xyz * _49.w;
    float3 sda = _63;
    float3 _69 = _49.xyz * _48.w;
    float3 dsa = _69;
    bool3 _75 = (_47.x != 0.0f).xxx;
    float3 _76 = float3(_75.x ? _69.x : _63.x, _75.y ? _69.y : _63.y, _75.z ? _69.z : _63.z);
    float3 l = _76;
    bool3 _81 = (_47.x != 0.0f).xxx;
    float3 _82 = float3(_81.x ? _63.x : _69.x, _81.y ? _63.y : _69.y, _81.z ? _63.z : _69.z);
    float3 r = _82;
    if (_47.y != 0.0f)
    {
        float _91 = _76.x;
        float _92 = _76.y;
        float _93 = _76.z;
        float _89 = min(min(_91, _92), _93);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _89;
        float _95 = max(max(_91, _92), _93);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _95;
        float3 _98 = 0.0f.xxx;
        if (_95 > _89)
        {
            float3 _104 = _82;
            _98 = ((_76 - _89.xxx) * blend_color_saturation_Qhh3(_104)) * (1.0f / (_95 - _89));
        }
        else
        {
            _98 = 0.0f.xxx;
        }
        l = _98;
        r = _69;
    }
    float _114 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_4_lum = _114;
    float3 _126 = (_114 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_5_result = _126;
    float _130 = _126.x;
    float _131 = _126.y;
    float _132 = _126.z;
    float _128 = min(min(_130, _131), _132);
    float _RESERVED_IDENTIFIER_FIXUP_6_minComp = _128;
    float _134 = max(max(_130, _131), _132);
    float _RESERVED_IDENTIFIER_FIXUP_7_maxComp = _134;
    bool _140 = false;
    if (_128 < 0.0f)
    {
        _140 = _114 != _128;
    }
    else
    {
        _140 = false;
    }
    if (_140)
    {
        float3 _143 = _114.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _143 + ((_126 - _143) * (_114 / ((_114 - _128) + _kGuardedDivideEpsilon)));
    }
    bool _155 = false;
    if (_134 > _57)
    {
        _155 = _134 != _114;
    }
    else
    {
        _155 = false;
    }
    if (_155)
    {
        float3 _159 = _114.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _159 + (((_RESERVED_IDENTIFIER_FIXUP_5_result - _159) * (_57 - _114)) * (1.0f / ((_134 - _114) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_5_result + _49.xyz) - _69) + _48.xyz) - _63, (_48.w + _49.w) - _57);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _192 = 0.0f.xx;
    float4 _198 = _18_src;
    float4 _202 = _18_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_192, _198, _202);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
