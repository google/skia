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
    float4 expected = float4(1.25f, 0.0f, 0.75f, 2.25f);
    bool _52 = false;
    if (abs(_11_testInputs.x) == 1.25f)
    {
        float2 _44 = abs(_11_testInputs.xy);
        _52 = all(bool2(_44.x == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.x, _44.y == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _64 = false;
    if (_52)
    {
        float3 _55 = abs(_11_testInputs.xyz);
        _64 = all(bool3(_55.x == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.x, _55.y == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.y, _55.z == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _64 = false;
    }
    bool _73 = false;
    if (_64)
    {
        float4 _67 = abs(_11_testInputs);
        _73 = all(bool4(_67.x == float4(1.25f, 0.0f, 0.75f, 2.25f).x, _67.y == float4(1.25f, 0.0f, 0.75f, 2.25f).y, _67.z == float4(1.25f, 0.0f, 0.75f, 2.25f).z, _67.w == float4(1.25f, 0.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _73 = false;
    }
    bool _77 = false;
    if (_73)
    {
        _77 = true;
    }
    else
    {
        _77 = false;
    }
    bool _84 = false;
    if (_77)
    {
        _84 = all(bool2(float2(1.25f, 0.0f).x == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.x, float2(1.25f, 0.0f).y == float4(1.25f, 0.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _84 = false;
    }
    bool _91 = false;
    if (_84)
    {
        _91 = all(bool3(float3(1.25f, 0.0f, 0.75f).x == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.x, float3(1.25f, 0.0f, 0.75f).y == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.y, float3(1.25f, 0.0f, 0.75f).z == float4(1.25f, 0.0f, 0.75f, 2.25f).xyz.z));
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
        _95 = _11_colorGreen;
    }
    else
    {
        _95 = _11_colorRed;
    }
    return _95;
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
