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

float guarded_divide_Qhhh(float _21, float _22)
{
    return _21 / (_22 + _kGuardedDivideEpsilon);
}

float color_dodge_component_Qhh2h2(float2 _32, float2 _33)
{
    float _43 = float((_33.x == 0.0f) ? 0 : 1);
    float dxScale = _43;
    float _56 = 0.0f;
    if (abs(_32.y - _32.x) >= 6.103515625e-05f)
    {
        float _65 = _33.x * _32.y;
        float _71 = _32.y - _32.x;
        _56 = guarded_divide_Qhhh(_65, _71);
    }
    else
    {
        _56 = _33.y;
    }
    float _76 = _43 * min(_33.y, _56);
    float delta = _76;
    return ((_76 * _32.y) + (_32.x * (1.0f - _33.y))) + (_33.x * (1.0f - _32.y));
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float2 _102 = _16_src.xw;
    float2 _106 = _16_dst.xw;
    float2 _111 = _16_src.yw;
    float2 _115 = _16_dst.yw;
    float2 _120 = _16_src.zw;
    float2 _124 = _16_dst.zw;
    sk_FragColor = float4(color_dodge_component_Qhh2h2(_102, _106), color_dodge_component_Qhh2h2(_111, _115), color_dodge_component_Qhh2h2(_120, _124), _16_src.w + ((1.0f - _16_src.w) * _16_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
