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

float _color_dodge_component_hh2h2(float2 _17, float2 _18)
{
    if (_18.x == 0.0f)
    {
        return _17.x * (1.0f - _18.y);
    }
    else
    {
        float delta = _17.y - _17.x;
        if (delta == 0.0f)
        {
            return ((_17.y * _18.y) + (_17.x * (1.0f - _18.y))) + (_18.x * (1.0f - _17.y));
        }
        else
        {
            delta = min(_18.y, (_18.x * _17.y) / delta);
            return ((delta * _17.y) + (_17.x * (1.0f - _18.y))) + (_18.x * (1.0f - _17.y));
        }
    }
}

void frag_main()
{
    float2 _102 = _11_src.xw;
    float2 _107 = _11_dst.xw;
    float2 _112 = _11_src.yw;
    float2 _116 = _11_dst.yw;
    float2 _121 = _11_src.zw;
    float2 _125 = _11_dst.zw;
    sk_FragColor = float4(_color_dodge_component_hh2h2(_102, _107), _color_dodge_component_hh2h2(_112, _116), _color_dodge_component_hh2h2(_121, _125), _11_src.w + ((1.0f - _11_src.w) * _11_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
