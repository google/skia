cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _21_src : packoffset(c0);
    float4 _21_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float blend_color_saturation_Qhh3(float3 _27)
{
    return max(max(_27.x, _27.y), _27.z) - min(min(_27.x, _27.y), _27.z);
}

float3 guarded_divide_Qh3h3h(float3 _48, float _49)
{
    return _48 * (1.0f / (_49 + _kGuardedDivideEpsilon));
}

float4 blend_hslc_h4h2h4h4(float2 _62, float4 _63, float4 _64)
{
    float _71 = _64.w * _63.w;
    float alpha = _71;
    float3 _77 = _63.xyz * _64.w;
    float3 sda = _77;
    float3 _83 = _64.xyz * _63.w;
    float3 dsa = _83;
    float3 _88 = 0.0f.xxx;
    if (_62.x != 0.0f)
    {
        _88 = _83;
    }
    else
    {
        _88 = _77;
    }
    float3 l = _88;
    float3 _97 = 0.0f.xxx;
    if (_62.x != 0.0f)
    {
        _97 = _77;
    }
    else
    {
        _97 = _83;
    }
    float3 r = _97;
    if (_62.y != 0.0f)
    {
        float _108 = min(min(_88.x, _88.y), _88.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _108;
        float _114 = max(max(_88.x, _88.y), _88.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _114;
        float _117 = _114 - _108;
        float _RESERVED_IDENTIFIER_FIXUP_4_diff = _117;
        float3 _120 = 0.0f.xxx;
        if (_117 >= 0.000244140625f)
        {
            float3 _126 = _97;
            float3 _129 = (_88 - _108.xxx) * blend_color_saturation_Qhh3(_126);
            float _130 = _117;
            _120 = guarded_divide_Qh3h3h(_129, _130);
        }
        else
        {
            _120 = 0.0f.xxx;
        }
        l = _120;
        r = _83;
    }
    float _135 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_5_lum = _135;
    float3 _147 = (_135 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_6_result = _147;
    float _151 = _147.x;
    float _152 = _147.y;
    float _153 = _147.z;
    float _149 = min(min(_151, _152), _153);
    float _RESERVED_IDENTIFIER_FIXUP_7_minComp = _149;
    float _155 = max(max(_151, _152), _153);
    float _RESERVED_IDENTIFIER_FIXUP_8_maxComp = _155;
    bool _161 = false;
    if (_149 < 0.0f)
    {
        _161 = _135 != _149;
    }
    else
    {
        _161 = false;
    }
    if (_161)
    {
        float3 _164 = _135.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _164 + ((_147 - _164) * (_135 / (((_135 - _149) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _178 = false;
    if (_155 > _71)
    {
        _178 = _155 != _135;
    }
    else
    {
        _178 = false;
    }
    if (_178)
    {
        float3 _182 = _135.xxx;
        _RESERVED_IDENTIFIER_FIXUP_6_result = _182 + (((_RESERVED_IDENTIFIER_FIXUP_6_result - _182) * (_71 - _135)) * (1.0f / (((_155 - _135) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_6_result + _64.xyz) - _83) + _63.xyz) - _77, (_63.w + _64.w) - _71);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _216 = 0.0f.xx;
    float4 _221 = _21_src;
    float4 _225 = _21_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_216, _221, _225);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
