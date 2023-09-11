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

float soft_light_component_Qhh2h2(float2 _21, float2 _22)
{
    if ((2.0f * _21.x) <= _21.y)
    {
        return ((((_22.x * _22.x) * (_21.y - (2.0f * _21.x))) / (_22.y + _kGuardedDivideEpsilon)) + ((1.0f - _22.y) * _21.x)) + (_22.x * (((-_21.y) + (2.0f * _21.x)) + 1.0f));
    }
    else
    {
        if ((4.0f * _22.x) <= _22.y)
        {
            float _87 = _22.x * _22.x;
            float DSqd = _87;
            float _91 = _87 * _22.x;
            float DCub = _91;
            float _97 = _22.y * _22.y;
            float DaSqd = _97;
            float _101 = _97 * _22.y;
            float DaCub = _101;
            return ((((_97 * (_21.x - (_22.x * (((3.0f * _21.y) - (6.0f * _21.x)) - 1.0f)))) + (((12.0f * _22.y) * _87) * (_21.y - (2.0f * _21.x)))) - ((16.0f * _91) * (_21.y - (2.0f * _21.x)))) - (_101 * _21.x)) / (_97 + _kGuardedDivideEpsilon);
        }
        else
        {
            return (((_22.x * ((_21.y - (2.0f * _21.x)) + 1.0f)) + _21.x) - (sqrt(_22.y * _22.x) * (_21.y - (2.0f * _21.x)))) - (_22.y * _21.x);
        }
    }
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float4 _192 = 0.0f.xxxx;
    if (_15_dst.w == 0.0f)
    {
        _192 = _15_src;
    }
    else
    {
        float2 _203 = _15_src.xw;
        float2 _207 = _15_dst.xw;
        float2 _212 = _15_src.yw;
        float2 _216 = _15_dst.yw;
        float2 _221 = _15_src.zw;
        float2 _225 = _15_dst.zw;
        _192 = float4(soft_light_component_Qhh2h2(_203, _207), soft_light_component_Qhh2h2(_212, _216), soft_light_component_Qhh2h2(_221, _225), _15_src.w + ((1.0f - _15_src.w) * _15_dst.w));
    }
    sk_FragColor = _192;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
