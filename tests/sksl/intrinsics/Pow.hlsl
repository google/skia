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
    float4 expected = float4(-1.5625f, 0.0f, 0.75f, 3.375f);
    bool _54 = false;
    if (pow(_10_testInputs.x, 2.0f) == (-1.5625f))
    {
        float2 _44 = pow(_10_testInputs.xy, float2(2.0f, 3.0f));
        _54 = all(bool2(_44.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, _44.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _54 = false;
    }
    bool _68 = false;
    if (_54)
    {
        float3 _57 = pow(_10_testInputs.xyz, float3(2.0f, 3.0f, 1.0f));
        _68 = all(bool3(_57.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, _57.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, _57.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _79 = false;
    if (_68)
    {
        float4 _71 = pow(_10_testInputs, float4(2.0f, 3.0f, 1.0f, 1.5f));
        _79 = all(bool4(_71.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, _71.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, _71.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, _71.w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _79 = false;
    }
    bool _84 = false;
    if (_79)
    {
        _84 = 1.5625f == (-1.5625f);
    }
    else
    {
        _84 = false;
    }
    bool _91 = false;
    if (_84)
    {
        _91 = all(bool2(float2(1.5625f, 0.0f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, float2(1.5625f, 0.0f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _91 = false;
    }
    bool _98 = false;
    if (_91)
    {
        _98 = all(bool3(float3(1.5625f, 0.0f, 0.75f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, float3(1.5625f, 0.0f, 0.75f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, float3(1.5625f, 0.0f, 0.75f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _98 = false;
    }
    bool _104 = false;
    if (_98)
    {
        _104 = all(bool4(float4(1.5625f, 0.0f, 0.75f, 3.375f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, float4(1.5625f, 0.0f, 0.75f, 3.375f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, float4(1.5625f, 0.0f, 0.75f, 3.375f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, float4(1.5625f, 0.0f, 0.75f, 3.375f).w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _104 = false;
    }
    float4 _105 = 0.0f.xxxx;
    if (_104)
    {
        _105 = _10_colorGreen;
    }
    else
    {
        _105 = _10_colorRed;
    }
    return _105;
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
