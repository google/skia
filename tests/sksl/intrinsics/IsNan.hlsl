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
    float4 valueIsNaN = 0.0f.xxxx / _10_testInputs.yyyy;
    float4 valueIsNumber = 1.0f.xxxx / _10_testInputs;
    bool _53 = false;
    if (isnan(valueIsNaN.x))
    {
        _53 = all(isnan(valueIsNaN.xy));
    }
    else
    {
        _53 = false;
    }
    bool _62 = false;
    if (_53)
    {
        _62 = all(isnan(valueIsNaN.xyz));
    }
    else
    {
        _62 = false;
    }
    bool _69 = false;
    if (_62)
    {
        _69 = all(isnan(valueIsNaN));
    }
    else
    {
        _69 = false;
    }
    bool _76 = false;
    if (_69)
    {
        _76 = !isnan(valueIsNumber.x);
    }
    else
    {
        _76 = false;
    }
    bool _84 = false;
    if (_76)
    {
        _84 = !any(isnan(valueIsNumber.xy));
    }
    else
    {
        _84 = false;
    }
    bool _92 = false;
    if (_84)
    {
        _92 = !any(isnan(valueIsNumber.xyz));
    }
    else
    {
        _92 = false;
    }
    bool _99 = false;
    if (_92)
    {
        _99 = !any(isnan(valueIsNumber));
    }
    else
    {
        _99 = false;
    }
    float4 _100 = 0.0f.xxxx;
    if (_99)
    {
        _100 = _10_colorGreen;
    }
    else
    {
        _100 = _10_colorRed;
    }
    return _100;
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
