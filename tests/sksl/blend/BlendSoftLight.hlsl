cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _19_src : packoffset(c0);
    float4 _19_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float _kGuardedDivideEpsilon = 0.0f;

float soft_light_component_Qhh2h2(float2 _25, float2 _26)
{
    if ((2.0f * _25.x) <= _25.y)
    {
        return ((((_26.x * _26.x) * (_25.y - (2.0f * _25.x))) / (_26.y + _kGuardedDivideEpsilon)) + ((1.0f - _26.y) * _25.x)) + (_26.x * (((-_25.y) + (2.0f * _25.x)) + 1.0f));
    }
    else
    {
        if ((4.0f * _26.x) <= _26.y)
        {
            float _91 = _26.x * _26.x;
            float DSqd = _91;
            float _95 = _91 * _26.x;
            float DCub = _95;
            float _101 = _26.y * _26.y;
            float DaSqd = _101;
            float _105 = _101 * _26.y;
            float DaCub = _105;
            return ((((_101 * (_25.x - (_26.x * (((3.0f * _25.y) - (6.0f * _25.x)) - 1.0f)))) + (((12.0f * _26.y) * _91) * (_25.y - (2.0f * _25.x)))) - ((16.0f * _95) * (_25.y - (2.0f * _25.x)))) - (_105 * _25.x)) / (_101 + _kGuardedDivideEpsilon);
        }
        else
        {
            return (((_26.x * ((_25.y - (2.0f * _25.x)) + 1.0f)) + _25.x) - (sqrt(_26.y * _26.x) * (_25.y - (2.0f * _25.x)))) - (_26.y * _25.x);
        }
    }
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float4 _195 = 0.0f.xxxx;
    if (_19_dst.w == 0.0f)
    {
        _195 = _19_src;
    }
    else
    {
        float2 _206 = _19_src.xw;
        float2 _210 = _19_dst.xw;
        float2 _215 = _19_src.yw;
        float2 _219 = _19_dst.yw;
        float2 _224 = _19_src.zw;
        float2 _228 = _19_dst.zw;
        _195 = float4(soft_light_component_Qhh2h2(_206, _210), soft_light_component_Qhh2h2(_215, _219), soft_light_component_Qhh2h2(_224, _228), _19_src.w + ((1.0f - _19_src.w) * _19_dst.w));
    }
    sk_FragColor = _195;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
