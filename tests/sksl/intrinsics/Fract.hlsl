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
    bool _46 = false;
    if (frac(_10_testInputs.x) == 0.75f)
    {
        float2 _38 = frac(_10_testInputs.xy);
        _46 = all(bool2(_38.x == float2(0.75f, 0.0f).x, _38.y == float2(0.75f, 0.0f).y));
    }
    else
    {
        _46 = false;
    }
    bool _58 = false;
    if (_46)
    {
        float3 _49 = frac(_10_testInputs.xyz);
        _58 = all(bool3(_49.x == float3(0.75f, 0.0f, 0.75f).x, _49.y == float3(0.75f, 0.0f, 0.75f).y, _49.z == float3(0.75f, 0.0f, 0.75f).z));
    }
    else
    {
        _58 = false;
    }
    bool _69 = false;
    if (_58)
    {
        float4 _61 = frac(_10_testInputs);
        _69 = all(bool4(_61.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _61.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _61.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _61.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
    }
    else
    {
        _69 = false;
    }
    float4 _70 = 0.0f.xxxx;
    if (_69)
    {
        _70 = _10_colorGreen;
    }
    else
    {
        _70 = _10_colorRed;
    }
    return _70;
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
