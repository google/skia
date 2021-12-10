cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _17_src : packoffset(c0);
    float4 _17_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float color_dodge_component_Qhh2h2(float2 _23, float2 _24)
{
    if (_24.x == 0.0f)
    {
        return _23.x * (1.0f - _24.y);
    }
    else
    {
        float _45 = _23.y - _23.x;
        float delta = _45;
        if (_45 == 0.0f)
        {
            return ((_23.y * _24.y) + (_23.x * (1.0f - _24.y))) + (_24.x * (1.0f - _23.y));
        }
        else
        {
            float _69 = min(_24.y, (_24.x * _23.y) / (_45 + _kGuardedDivideEpsilon));
            delta = _69;
            return ((_69 * _23.y) + (_23.x * (1.0f - _24.y))) + (_24.x * (1.0f - _23.y));
        }
    }
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _106 = _17_src.xw;
    float2 _111 = _17_dst.xw;
    float2 _116 = _17_src.yw;
    float2 _120 = _17_dst.yw;
    float2 _125 = _17_src.zw;
    float2 _129 = _17_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_106, _111), color_dodge_component_Qhh2h2(_116, _120), color_dodge_component_Qhh2h2(_125, _129), _17_src.w + ((1.0f - _17_src.w) * _17_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
