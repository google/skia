cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _20_src : packoffset(c0);
    float4 _20_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float blend_color_saturation_Qhh3(float3 _26)
{
    return max(max(_26.x, _26.y), _26.z) - min(min(_26.x, _26.y), _26.z);
}

float4 blend_hslc_h4h2h4h4(float2 _49, float4 _50, float4 _51)
{
    float _59 = _51.w * _50.w;
    float alpha = _59;
    float3 _65 = _50.xyz * _51.w;
    float3 sda = _65;
    float3 _71 = _51.xyz * _50.w;
    float3 dsa = _71;
    float3 _76 = 0.0f.xxx;
    if (_49.x != 0.0f)
    {
        _76 = _71;
    }
    else
    {
        _76 = _65;
    }
    float3 l = _76;
    float3 _85 = 0.0f.xxx;
    if (_49.x != 0.0f)
    {
        _85 = _65;
    }
    else
    {
        _85 = _71;
    }
    float3 r = _85;
    if (_49.y != 0.0f)
    {
        float _96 = min(min(_76.x, _76.y), _76.z);
        float _RESERVED_IDENTIFIER_FIXUP_2_mn = _96;
        float _102 = max(max(_76.x, _76.y), _76.z);
        float _RESERVED_IDENTIFIER_FIXUP_3_mx = _102;
        float3 _105 = 0.0f.xxx;
        if (_102 > _96)
        {
            float3 _111 = _85;
            _105 = ((_76 - _96.xxx) * blend_color_saturation_Qhh3(_111)) * (1.0f / (_102 - _96));
        }
        else
        {
            _105 = 0.0f.xxx;
        }
        l = _105;
        r = _71;
    }
    float _121 = dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), r);
    float _RESERVED_IDENTIFIER_FIXUP_4_lum = _121;
    float3 _133 = (_121 - dot(float3(0.300000011920928955078125f, 0.589999973773956298828125f, 0.10999999940395355224609375f), l)).xxx + l;
    float3 _RESERVED_IDENTIFIER_FIXUP_5_result = _133;
    float _137 = _133.x;
    float _138 = _133.y;
    float _139 = _133.z;
    float _135 = min(min(_137, _138), _139);
    float _RESERVED_IDENTIFIER_FIXUP_6_minComp = _135;
    float _141 = max(max(_137, _138), _139);
    float _RESERVED_IDENTIFIER_FIXUP_7_maxComp = _141;
    bool _147 = false;
    if (_135 < 0.0f)
    {
        _147 = _121 != _135;
    }
    else
    {
        _147 = false;
    }
    if (_147)
    {
        float3 _150 = _121.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _150 + ((_133 - _150) * (_121 / (((_121 - _135) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    bool _164 = false;
    if (_141 > _59)
    {
        _164 = _141 != _121;
    }
    else
    {
        _164 = false;
    }
    if (_164)
    {
        float3 _168 = _121.xxx;
        _RESERVED_IDENTIFIER_FIXUP_5_result = _168 + (((_RESERVED_IDENTIFIER_FIXUP_5_result - _168) * (_59 - _121)) * (1.0f / (((_141 - _121) + 6.103515625e-05f) + _kGuardedDivideEpsilon)));
    }
    return float4((((_RESERVED_IDENTIFIER_FIXUP_5_result + _51.xyz) - _71) + _50.xyz) - _65, (_50.w + _51.w) - _59);
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _202 = 0.0f.xx;
    float4 _207 = _20_src;
    float4 _211 = _20_dst;
    sk_FragColor = blend_hslc_h4h2h4h4(_202, _207, _211);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
