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
    float3 _74 = 0.0f.xxx;
    if (_47.x != 0.0f)
    {
        _74 = _69;
    }
    else
    {
        _74 = _63;
    }
    float3 l = _74;
    float3 _83 = 0.0f.xxx;
    if (_47.x != 0.0f)
    {
        _83 = _63;
    }
    else
    {
        _83 = _69;
    }
    float3 r = _83;
    if (_47.y != 0.0f)
    {
        float _94 = min(min(_74.x, _74.y), _74.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _94;
        float _100 = max(max(_74.x, _74.y), _74.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _100;
        float3 _103 = 0.0f.xxx;
        if (_100 > _94)
        {
            float3 _109 = _83;
            _103 = ((_74 - _94.xxx) * blend_color_saturation_Qhh3(_109)) * (1.0f / (_100 - _94));
        }
        else
        {
            _103 = 0.0f.xxx;
        }
        l = _103;
        r = _69;
    }
    float _119 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_4_lum = _119;
    float3 _131 = (_119 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_5_result = _131;
    float _135 = _131.x;
    float _136 = _131.y;
    float _137 = _131.z;
    float _133 = min(min(_135, _136), _137);
    float _RESERVED_IDENTIFIER_FIXUP_6_minComp = _133;
    float _139 = max(max(_135, _136), _137);
    float _RESERVED_IDENTIFIER_FIXUP_7_maxComp = _139;
    bool _145 = false;
    if (_133 < 0.0f)
    {
        _145 = _119 != _133;
    }
    else
    {
        _145 = false;
    }
    if (_145)
    {
        float3 _148 = _119.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _148 + ((_131 - _148) * (_119 / ((_119 - _133) + _kGuardedDivideEpsilon)));
    }
    bool _160 = false;
    if (_139 > _57)
    {
        _160 = _139 != _119;
    }
    else
    {
        _160 = false;
    }
    if (_160)
    {
        float3 _164 = _119.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _164 + (((_RESERVED_IDENTIFIER_FIXUP_5_result - _164) * (_57 - _119)) * (1.0f / ((_139 - _119) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_5_result + _49.xyz) - _69) + _48.xyz) - _63, (_48.w + _49.w) - _57);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _197 = 1.0f.xx;
    float4 _203 = _18_src;
    float4 _207 = _18_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_197, _203, _207);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
