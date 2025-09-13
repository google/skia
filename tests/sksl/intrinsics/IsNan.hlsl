cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _35 = 0.0f.xxxx / _11_testInputs.yyyy;
    float4 valueIsNaN = _35;
    float4 _41 = 1.0f.xxxx / _11_testInputs;
    float4 valueIsNumber = _41;
    bool _52 = false;
    if (isnan(_35.x))
    {
        _52 = all(isnan(_35.xy));
    }
    else
    {
        _52 = false;
    }
    bool _60 = false;
    if (_52)
    {
        _60 = all(isnan(_35.xyz));
    }
    else
    {
        _60 = false;
    }
    bool _66 = false;
    if (_60)
    {
        _66 = all(isnan(_35));
    }
    else
    {
        _66 = false;
    }
    bool _72 = false;
    if (_66)
    {
        _72 = !isnan(_41.x);
    }
    else
    {
        _72 = false;
    }
    bool _79 = false;
    if (_72)
    {
        _79 = !any(isnan(_41.xy));
    }
    else
    {
        _79 = false;
    }
    bool _86 = false;
    if (_79)
    {
        _86 = !any(isnan(_41.xyz));
    }
    else
    {
        _86 = false;
    }
    bool _92 = false;
    if (_86)
    {
        _92 = !any(isnan(_41));
    }
    else
    {
        _92 = false;
    }
    float4 _93 = 0.0f.xxxx;
    if (_92)
    {
        _93 = _11_colorGreen;
    }
    else
    {
        _93 = _11_colorRed;
    }
    return _93;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
