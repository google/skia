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

float soft_light_component_Qhh2h2(float2 _23, float2 _24)
{
    if ((2.0f * _23.x) <= _23.y)
    {
        return ((((_24.x * _24.x) * (_23.y - (2.0f * _23.x))) / (_24.y + _kGuardedDivideEpsilon)) + ((1.0f - _24.y) * _23.x)) + (_24.x * (((-_23.y) + (2.0f * _23.x)) + 1.0f));
    }
    else
    {
        if ((4.0f * _24.x) <= _24.y)
        {
            float _89 = _24.x * _24.x;
            float DSqd = _89;
            float _93 = _89 * _24.x;
            float DCub = _93;
            float _99 = _24.y * _24.y;
            float DaSqd = _99;
            float _103 = _99 * _24.y;
            float DaCub = _103;
            return ((((_99 * (_23.x - (_24.x * (((3.0f * _23.y) - (6.0f * _23.x)) - 1.0f)))) + (((12.0f * _24.y) * _89) * (_23.y - (2.0f * _23.x)))) - ((16.0f * _93) * (_23.y - (2.0f * _23.x)))) - (_103 * _23.x)) / (_99 + _kGuardedDivideEpsilon);
        }
        else
        {
            return (((_24.x * ((_23.y - (2.0f * _23.x)) + 1.0f)) + _23.x) - (sqrt(_24.y * _24.x) * (_23.y - (2.0f * _23.x)))) - (_24.y * _23.x);
        }
    }
}

void frag_main()
{
    _kGuardedDivideEpsilon = false ? 9.9999999392252902907785028219223e-09f : 0.0f;
    float4 _194 = 0.0f.xxxx;
    if (_17_dst.w == 0.0f)
    {
        _194 = _17_src;
    }
    else
    {
        float2 _205 = _17_src.xw;
        float2 _209 = _17_dst.xw;
        float2 _214 = _17_src.yw;
        float2 _218 = _17_dst.yw;
        float2 _223 = _17_src.zw;
        float2 _227 = _17_dst.zw;
        _194 = float4(soft_light_component_Qhh2h2(_205, _209), soft_light_component_Qhh2h2(_214, _218), soft_light_component_Qhh2h2(_223, _227), _17_src.w + ((1.0f - _17_src.w) * _17_dst.w));
    }
    sk_FragColor = _194;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
