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
    float4 expected = float4(0.0f, 0.0f, 0.75f, 1.0f);
    bool _49 = false;
    if (clamp(_7_testInputs.x, 0.0f, 1.0f) == 0.0f)
    {
        float2 _40 = clamp(_7_testInputs.xy, 0.0f.xx, 1.0f.xx);
        _49 = all(bool2(_40.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, _40.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _49 = false;
    }
    bool _63 = false;
    if (_49)
    {
        float3 _52 = clamp(_7_testInputs.xyz, 0.0f.xxx, 1.0f.xxx);
        _63 = all(bool3(_52.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, _52.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, _52.z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _74 = false;
    if (_63)
    {
        float4 _66 = clamp(_7_testInputs, 0.0f.xxxx, 1.0f.xxxx);
        _74 = all(bool4(_66.x == float4(0.0f, 0.0f, 0.75f, 1.0f).x, _66.y == float4(0.0f, 0.0f, 0.75f, 1.0f).y, _66.z == float4(0.0f, 0.0f, 0.75f, 1.0f).z, _66.w == float4(0.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _74 = false;
    }
    bool _78 = false;
    if (_74)
    {
        _78 = true;
    }
    else
    {
        _78 = false;
    }
    bool _84 = false;
    if (_78)
    {
        _84 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _84 = false;
    }
    bool _91 = false;
    if (_84)
    {
        _91 = all(bool3(float3(0.0f, 0.0f, 0.75f).x == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.75f).y == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.75f).z == float4(0.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _91 = false;
    }
    bool _94 = false;
    if (_91)
    {
        _94 = true;
    }
    else
    {
        _94 = false;
    }
    float4 _95 = 0.0f.xxxx;
    if (_94)
    {
        _95 = _7_colorGreen;
    }
    else
    {
        _95 = _7_colorRed;
    }
    return _95;
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
