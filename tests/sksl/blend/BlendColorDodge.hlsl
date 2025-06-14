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

float guarded_divide_Qhhh(float _25, float _26)
{
    return _25 / (_26 + _kGuardedDivideEpsilon);
}

float color_dodge_component_Qhh2h2(float2 _36, float2 _37)
{
    float _46 = float((_37.x == 0.0f) ? 0 : 1);
    float dxScale = _46;
    float _59 = 0.0f;
    if (abs(_36.y - _36.x) >= 6.103515625e-05f)
    {
        float _68 = _37.x * _36.y;
        float _74 = _36.y - _36.x;
        _59 = guarded_divide_Qhhh(_68, _74);
    }
    else
    {
        _59 = _37.y;
    }
    float _79 = _46 * min(_37.y, _59);
    float delta = _79;
    return ((_79 * _36.y) + (_36.x * (1.0f - _37.y))) + (_37.x * (1.0f - _36.y));
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _105 = _20_src.xw;
    float2 _109 = _20_dst.xw;
    float2 _114 = _20_src.yw;
    float2 _118 = _20_dst.yw;
    float2 _123 = _20_src.zw;
    float2 _127 = _20_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_105, _109), color_dodge_component_Qhh2h2(_114, _118), color_dodge_component_Qhh2h2(_123, _127), _20_src.w + ((1.0f - _20_src.w) * _20_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
