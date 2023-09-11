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
    float4 expected = float4(1.25f, 0.0f, 0.75f, 2.25f);
    bool _49 = false;
    if (abs(_7_testInputs.x) == 1.25f)
    {
        float2 _41 = abs(_7_testInputs.xy);
        _49 = all(bool2(_41.x == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.x, _41.y == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _61 = false;
    if (_49)
    {
        float3 _52 = abs(_7_testInputs.xyz);
        _61 = all(bool3(_52.x == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.x, _52.y == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.y, _52.z == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _61 = false;
    }
    bool _70 = false;
    if (_61)
    {
        float4 _64 = abs(_7_testInputs);
        _70 = all(bool4(_64.x == float4(1.25f, 0.0f, 0.75f, 2.25f).x, _64.y == float4(1.25f, 0.0f, 0.75f, 2.25f).y, _64.z == float4(1.25f, 0.0f, 0.75f, 2.25f).z, _64.w == float4(1.25f, 0.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _70 = false;
    }
    bool _74 = false;
    if (_70)
    {
        _74 = true;
    }
    else
    {
        _74 = false;
    }
    bool _81 = false;
    if (_74)
    {
        _81 = all(bool2(float2(1.25f, 0.0f).x == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.x, float2(1.25f, 0.0f).y == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _81 = false;
    }
    bool _88 = false;
    if (_81)
    {
        _88 = all(bool3(float3(1.25f, 0.0f, 0.75f).x == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.x, float3(1.25f, 0.0f, 0.75f).y == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.y, float3(1.25f, 0.0f, 0.75f).z == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _88 = false;
    }
    bool _91 = false;
    if (_88)
    {
        _91 = true;
    }
    else
    {
        _91 = false;
    }
    float4 _92 = 0.0f.xxxx;
    if (_91)
    {
        _92 = _7_colorGreen;
    }
    else
    {
        _92 = _7_colorRed;
    }
    return _92;
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
