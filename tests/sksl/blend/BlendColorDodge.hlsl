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
    float _33 = float((_22.x == 0.0f) ? 0 : 1);
    float dxScale = _33;
    float _84 = _33 * min(_22.y, (abs(_21.y - _21.x) >= 6.103515625e-05f) ? ((_22.x * _21.y) / ((_21.y - _21.x) + _kGuardedDivideEpsilon)) : _22.y);
    float delta = _84;
    return ((_84 * _21.y) + (_21.x * (1.0f - _22.y))) + (_22.x * (1.0f - _21.y));
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _110 = _15_src.xw;
    float2 _114 = _15_dst.xw;
    float2 _119 = _15_src.yw;
    float2 _123 = _15_dst.yw;
    float2 _128 = _15_src.zw;
    float2 _132 = _15_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_110, _114), color_dodge_component_Qhh2h2(_119, _123), color_dodge_component_Qhh2h2(_128, _132), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
