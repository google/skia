cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _32 = 0.0f.xxxx / _7_testInputs.yyyy;
    float4 valueIsNaN = _32;
    float4 _38 = 1.0f.xxxx / _7_testInputs;
    float4 valueIsNumber = _38;
    bool _49 = false;
    if (isnan(_32.x))
    {
        _49 = all(isnan(_32.xy));
    }
    else
    {
        _49 = false;
    }
    bool _57 = false;
    if (_49)
    {
        _57 = all(isnan(_32.xyz));
    }
    else
    {
        _57 = false;
    }
    bool _63 = false;
    if (_57)
    {
        _63 = all(isnan(_32));
    }
    else
    {
        _63 = false;
    }
    bool _69 = false;
    if (_63)
    {
        _69 = !isnan(_38.x);
    }
    else
    {
        _69 = false;
    }
    bool _76 = false;
    if (_69)
    {
        _76 = !any(isnan(_38.xy));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        _83 = !any(isnan(_38.xyz));
    }
    else
    {
        _83 = false;
    }
    bool _89 = false;
    if (_83)
    {
        _89 = !any(isnan(_38));
    }
    else
    {
        _89 = false;
    }
    float4 _90 = 0.0f.xxxx;
    if (_89)
    {
        _90 = _7_colorGreen;
    }
    else
    {
        _90 = _7_colorRed;
    }
    return _90;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
