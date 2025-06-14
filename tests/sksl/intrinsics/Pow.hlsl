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
    float4 expected = float4(-1.5625f, 0.0f, 0.75f, 3.375f);
    bool _55 = false;
    if (pow(_11_testInputs.x, 2.0f) == (-1.5625f))
    {
        float2 _45 = pow(_11_testInputs.xy, float2(2.0f, 3.0f));
        _55 = all(bool2(_45.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, _45.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _69 = false;
    if (_55)
    {
        float3 _58 = pow(_11_testInputs.xyz, float3(2.0f, 3.0f, 1.0f));
        _69 = all(bool3(_58.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, _58.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, _58.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _69 = false;
    }
    bool _80 = false;
    if (_69)
    {
        float4 _72 = pow(_11_testInputs, float4(2.0f, 3.0f, 1.0f, 1.5f));
        _80 = all(bool4(_72.x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, _72.y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, _72.z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, _72.w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _80 = false;
    }
    bool _85 = false;
    if (_80)
    {
        _85 = 1.5625f == (-1.5625f);
    }
    else
    {
        _85 = false;
    }
    bool _92 = false;
    if (_85)
    {
        _92 = all(bool2(float2(1.5625f, 0.0f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.x, float2(1.5625f, 0.0f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xy.y));
    }
    else
    {
        _92 = false;
    }
    bool _99 = false;
    if (_92)
    {
        _99 = all(bool3(float3(1.5625f, 0.0f, 0.75f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.x, float3(1.5625f, 0.0f, 0.75f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.y, float3(1.5625f, 0.0f, 0.75f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).xyz.z));
    }
    else
    {
        _99 = false;
    }
    bool _105 = false;
    if (_99)
    {
        _105 = all(bool4(float4(1.5625f, 0.0f, 0.75f, 3.375f).x == float4(-1.5625f, 0.0f, 0.75f, 3.375f).x, float4(1.5625f, 0.0f, 0.75f, 3.375f).y == float4(-1.5625f, 0.0f, 0.75f, 3.375f).y, float4(1.5625f, 0.0f, 0.75f, 3.375f).z == float4(-1.5625f, 0.0f, 0.75f, 3.375f).z, float4(1.5625f, 0.0f, 0.75f, 3.375f).w == float4(-1.5625f, 0.0f, 0.75f, 3.375f).w));
    }
    else
    {
        _105 = false;
    }
    float4 _106 = 0.0f.xxxx;
    if (_105)
    {
        _106 = _11_colorGreen;
    }
    else
    {
        _106 = _11_colorRed;
    }
    return _106;
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
