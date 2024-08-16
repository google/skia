cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_src : packoffset(c0);
    float4 _15_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float color_dodge_component_Qhh2h2(float2 _21, float2 _22)
{
    if (_22.x == 0.0f)
    {
        return _21.x * (1.0f - _22.y);
    }
    else
    {
        float _43 = _21.y - _21.x;
        float delta = _43;
        if (_43 == 0.0f)
        {
            return ((_21.y * _22.y) + (_21.x * (1.0f - _22.y))) + (_22.x * (1.0f - _21.y));
        }
        else
        {
            float _67 = min(_22.y, (_22.x * _21.y) / (_43 + _kGuardedDivideEpsilon));
            delta = _67;
            return ((_67 * _21.y) + (_21.x * (1.0f - _22.y))) + (_22.x * (1.0f - _21.y));
        }
    }
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _104 = _15_src.xw;
    float2 _109 = _15_dst.xw;
    float2 _114 = _15_src.yw;
    float2 _118 = _15_dst.yw;
    float2 _123 = _15_src.zw;
    float2 _127 = _15_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_104, _109), color_dodge_component_Qhh2h2(_114, _118), color_dodge_component_Qhh2h2(_123, _127), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
