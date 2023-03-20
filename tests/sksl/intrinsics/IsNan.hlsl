cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = 0.0f.xxxx / _10_testInputs.yyyy;
    float4 valueIsNaN = _35;
    float4 _41 = 1.0f.xxxx / _10_testInputs;
    float4 valueIsNumber = _41;
    bool _51 = false;
    if (isnan(_35.x))
    {
        _51 = all(isnan(_35.xy));
    }
    else
    {
        _51 = false;
    }
    bool _59 = false;
    if (_51)
    {
        _59 = all(isnan(_35.xyz));
    }
    else
    {
        _59 = false;
    }
    bool _65 = false;
    if (_59)
    {
        _65 = all(isnan(_35));
    }
    else
    {
        _65 = false;
    }
    bool _71 = false;
    if (_65)
    {
        _71 = !isnan(_41.x);
    }
    else
    {
        _71 = false;
    }
    bool _78 = false;
    if (_71)
    {
        _78 = !any(isnan(_41.xy));
    }
    else
    {
        _78 = false;
    }
    bool _85 = false;
    if (_78)
    {
        _85 = !any(isnan(_41.xyz));
    }
    else
    {
        _85 = false;
    }
    bool _91 = false;
    if (_85)
    {
        _91 = !any(isnan(_41));
    }
    else
    {
        _91 = false;
    }
    float4 _92 = 0.0f.xxxx;
    if (_91)
    {
        _92 = _10_colorGreen;
    }
    else
    {
        _92 = _10_colorRed;
    }
    return _92;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
