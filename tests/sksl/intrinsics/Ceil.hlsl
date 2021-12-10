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
    float4 expected = float4(-1.0f, 0.0f, 1.0f, 3.0f);
    bool _54 = false;
    if (ceil(_10_testInputs.x) == expected.x)
    {
        float2 _45 = ceil(_10_testInputs.xy);
        _54 = all(bool2(_45.x == expected.xy.x, _45.y == expected.xy.y));
    }
    else
    {
        _54 = false;
    }
    bool _67 = false;
    if (_54)
    {
        float3 _57 = ceil(_10_testInputs.xyz);
        _67 = all(bool3(_57.x == expected.xyz.x, _57.y == expected.xyz.y, _57.z == expected.xyz.z));
    }
    else
    {
        _67 = false;
    }
    bool _77 = false;
    if (_67)
    {
        float4 _70 = ceil(_10_testInputs);
        _77 = all(bool4(_70.x == expected.x, _70.y == expected.y, _70.z == expected.z, _70.w == expected.w));
    }
    else
    {
        _77 = false;
    }
    bool _83 = false;
    if (_77)
    {
        _83 = (-1.0f) == expected.x;
    }
    else
    {
        _83 = false;
    }
    bool _91 = false;
    if (_83)
    {
        _91 = all(bool2(float2(-1.0f, 0.0f).x == expected.xy.x, float2(-1.0f, 0.0f).y == expected.xy.y));
    }
    else
    {
        _91 = false;
    }
    bool _99 = false;
    if (_91)
    {
        _99 = all(bool3(float3(-1.0f, 0.0f, 1.0f).x == expected.xyz.x, float3(-1.0f, 0.0f, 1.0f).y == expected.xyz.y, float3(-1.0f, 0.0f, 1.0f).z == expected.xyz.z));
    }
    else
    {
        _99 = false;
    }
    bool _105 = false;
    if (_99)
    {
        _105 = all(bool4(float4(-1.0f, 0.0f, 1.0f, 3.0f).x == expected.x, float4(-1.0f, 0.0f, 1.0f, 3.0f).y == expected.y, float4(-1.0f, 0.0f, 1.0f, 3.0f).z == expected.z, float4(-1.0f, 0.0f, 1.0f, 3.0f).w == expected.w));
    }
    else
    {
        _105 = false;
    }
    float4 _106 = 0.0f.xxxx;
    if (_105)
    {
        _106 = _10_colorGreen;
    }
    else
    {
        _106 = _10_colorRed;
    }
    return _106;
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
