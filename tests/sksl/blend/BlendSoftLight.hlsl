cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_src : packoffset(c0);
    float4 _11_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float _soft_light_component_hh2h2(float2 _17, float2 _18)
{
    if ((2.0f * _17.x) <= _17.y)
    {
        return ((((_18.x * _18.x) * (_17.y - (2.0f * _17.x))) / _18.y) + ((1.0f - _18.y) * _17.x)) + (_18.x * (((-_17.y) + (2.0f * _17.x)) + 1.0f));
    }
    else
    {
        if ((4.0f * _18.x) <= _18.y)
        {
            float DSqd = _18.x * _18.x;
            float DCub = DSqd * _18.x;
            float DaSqd = _18.y * _18.y;
            float DaCub = DaSqd * _18.y;
            return ((((DaSqd * (_17.x - (_18.x * (((3.0f * _17.y) - (6.0f * _17.x)) - 1.0f)))) + (((12.0f * _18.y) * DSqd) * (_17.y - (2.0f * _17.x)))) - ((16.0f * DCub) * (_17.y - (2.0f * _17.x)))) - (DaCub * _17.x)) / DaSqd;
        }
        else
        {
            return (((_18.x * ((_17.y - (2.0f * _17.x)) + 1.0f)) + _17.x) - (sqrt(_18.y * _18.x) * (_17.y - (2.0f * _17.x)))) - (_18.y * _17.x);
        }
    }
}

void frag_main()
{
    float4 _192 = 0.0f.xxxx;
    if (_11_dst.w == 0.0f)
    {
        _192 = _11_src;
    }
    else
    {
        float2 _203 = _11_src.xw;
        float2 _207 = _11_dst.xw;
        float2 _212 = _11_src.yw;
        float2 _216 = _11_dst.yw;
        float2 _221 = _11_src.zw;
        float2 _225 = _11_dst.zw;
        _192 = float4(_soft_light_component_hh2h2(_203, _207), _soft_light_component_hh2h2(_212, _216), _soft_light_component_hh2h2(_221, _225), _11_src.w + ((1.0f - _11_src.w) * _11_dst.w));
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
