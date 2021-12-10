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

float _color_burn_component_hh2h2(float2 _17, float2 _18)
{
    if (_18.y == _18.x)
    {
        return ((_17.y * _18.y) + (_17.x * (1.0f - _18.y))) + (_18.x * (1.0f - _17.y));
    }
    else
    {
        if (_17.x == 0.0f)
        {
            return _18.x * (1.0f - _17.y);
        }
        else
        {
            float delta = max(0.0f, _18.y - (((_18.y - _18.x) * _17.y) / _17.x));
            return ((delta * _17.y) + (_17.x * (1.0f - _18.y))) + (_18.x * (1.0f - _17.y));
        }
    }
}

void frag_main()
{
    float2 _105 = _11_src.xw;
    float2 _110 = _11_dst.xw;
    float2 _115 = _11_src.yw;
    float2 _119 = _11_dst.yw;
    float2 _124 = _11_src.zw;
    float2 _128 = _11_dst.zw;
    sk_FragColor = float4(_color_burn_component_hh2h2(_105, _110), _color_burn_component_hh2h2(_115, _119), _color_burn_component_hh2h2(_124, _128), _11_src.w + ((1.0f - _11_src.w) * _11_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
