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
    float4 expectedA = float4(-1.0f, 0.0f, 0.75f, 1.0f);
    float4 expectedB = float4(-1.0f, 0.0f, 0.5f, 2.25f);
    bool _58 = false;
    if (clamp(_11_testInputs.x, -1.0f, 1.0f) == (-1.0f))
    {
        float2 _48 = clamp(_11_testInputs.xy, (-1.0f).xx, 1.0f.xx);
        _58 = all(bool2(_48.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, _48.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _58 = false;
    }
    bool _72 = false;
    if (_58)
    {
        float3 _61 = clamp(_11_testInputs.xyz, (-1.0f).xxx, 1.0f.xxx);
        _72 = all(bool3(_61.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, _61.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, _61.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _72 = false;
    }
    bool _83 = false;
    if (_72)
    {
        float4 _75 = clamp(_11_testInputs, (-1.0f).xxxx, 1.0f.xxxx);
        _83 = all(bool4(_75.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).x, _75.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).y, _75.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).z, _75.w == float4(-1.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _83 = false;
    }
    bool _91 = false;
    if (_83)
    {
        _91 = clamp(_11_testInputs.x, -1.0f, 1.0f) == (-1.0f);
    }
    else
    {
        _91 = false;
    }
    bool _105 = false;
    if (_91)
    {
        float2 _94 = clamp(_11_testInputs.xy, float2(-1.0f, -2.0f), float2(1.0f, 2.0f));
        _105 = all(bool2(_94.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, _94.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _105 = false;
    }
    bool _117 = false;
    if (_105)
    {
        float3 _108 = clamp(_11_testInputs.xyz, float3(-1.0f, -2.0f, -2.0f), float3(1.0f, 2.0f, 0.5f));
        _117 = all(bool3(_108.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, _108.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, _108.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
    }
    else
    {
        _117 = false;
    }
    bool _128 = false;
    if (_117)
    {
        float4 _120 = clamp(_11_testInputs, float4(-1.0f, -2.0f, -2.0f, 1.0f), float4(1.0f, 2.0f, 0.5f, 3.0f));
        _128 = all(bool4(_120.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).x, _120.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).y, _120.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).z, _120.w == float4(-1.0f, 0.0f, 0.5f, 2.25f).w));
    }
    else
    {
        _128 = false;
    }
    bool _132 = false;
    if (_128)
    {
        _132 = true;
    }
    else
    {
        _132 = false;
    }
    bool _139 = false;
    if (_132)
    {
        _139 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _139 = false;
    }
    bool _146 = false;
    if (_139)
    {
        _146 = all(bool3(float3(-1.0f, 0.0f, 0.75f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(-1.0f, 0.0f, 0.75f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(-1.0f, 0.0f, 0.75f).z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _146 = false;
    }
    bool _149 = false;
    if (_146)
    {
        _149 = true;
    }
    else
    {
        _149 = false;
    }
    bool _152 = false;
    if (_149)
    {
        _152 = true;
    }
    else
    {
        _152 = false;
    }
    bool _158 = false;
    if (_152)
    {
        _158 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _158 = false;
    }
    bool _165 = false;
    if (_158)
    {
        _165 = all(bool3(float3(-1.0f, 0.0f, 0.5f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, float3(-1.0f, 0.0f, 0.5f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, float3(-1.0f, 0.0f, 0.5f).z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
    }
    else
    {
        _165 = false;
    }
    bool _168 = false;
    if (_165)
    {
        _168 = true;
    }
    else
    {
        _168 = false;
    }
    float4 _169 = 0.0f.xxxx;
    if (_168)
    {
        _169 = _11_colorGreen;
    }
    else
    {
        _169 = _11_colorRed;
    }
    return _169;
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
