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
    float4 expectedA = float4(-1.0f, 0.0f, 0.75f, 1.0f);
    float4 clampLow = float4(-1.0f, -2.0f, -2.0f, 1.0f);
    float4 expectedB = float4(-1.0f, 0.0f, 0.5f, 2.25f);
    float4 clampHigh = float4(1.0f, 2.0f, 0.5f, 3.0f);
    bool _67 = false;
    if (clamp(_10_testInputs.x, -1.0f, 1.0f) == expectedA.x)
    {
        float2 _56 = clamp(_10_testInputs.xy, (-1.0f).xx, 1.0f.xx);
        _67 = all(bool2(_56.x == expectedA.xy.x, _56.y == expectedA.xy.y));
    }
    else
    {
        _67 = false;
    }
    bool _82 = false;
    if (_67)
    {
        float3 _70 = clamp(_10_testInputs.xyz, (-1.0f).xxx, 1.0f.xxx);
        _82 = all(bool3(_70.x == expectedA.xyz.x, _70.y == expectedA.xyz.y, _70.z == expectedA.xyz.z));
    }
    else
    {
        _82 = false;
    }
    bool _94 = false;
    if (_82)
    {
        float4 _85 = clamp(_10_testInputs, (-1.0f).xxxx, 1.0f.xxxx);
        _94 = all(bool4(_85.x == expectedA.x, _85.y == expectedA.y, _85.z == expectedA.z, _85.w == expectedA.w));
    }
    else
    {
        _94 = false;
    }
    bool _100 = false;
    if (_94)
    {
        _100 = (-1.0f) == expectedA.x;
    }
    else
    {
        _100 = false;
    }
    bool _108 = false;
    if (_100)
    {
        _108 = all(bool2(float2(-1.0f, 0.0f).x == expectedA.xy.x, float2(-1.0f, 0.0f).y == expectedA.xy.y));
    }
    else
    {
        _108 = false;
    }
    bool _116 = false;
    if (_108)
    {
        _116 = all(bool3(float3(-1.0f, 0.0f, 0.75f).x == expectedA.xyz.x, float3(-1.0f, 0.0f, 0.75f).y == expectedA.xyz.y, float3(-1.0f, 0.0f, 0.75f).z == expectedA.xyz.z));
    }
    else
    {
        _116 = false;
    }
    bool _122 = false;
    if (_116)
    {
        _122 = all(bool4(float4(-1.0f, 0.0f, 0.75f, 1.0f).x == expectedA.x, float4(-1.0f, 0.0f, 0.75f, 1.0f).y == expectedA.y, float4(-1.0f, 0.0f, 0.75f, 1.0f).z == expectedA.z, float4(-1.0f, 0.0f, 0.75f, 1.0f).w == expectedA.w));
    }
    else
    {
        _122 = false;
    }
    bool _132 = false;
    if (_122)
    {
        _132 = clamp(_10_testInputs.x, -1.0f, 1.0f) == expectedB.x;
    }
    else
    {
        _132 = false;
    }
    bool _145 = false;
    if (_132)
    {
        float2 _135 = clamp(_10_testInputs.xy, float2(-1.0f, -2.0f), float2(1.0f, 2.0f));
        _145 = all(bool2(_135.x == expectedB.xy.x, _135.y == expectedB.xy.y));
    }
    else
    {
        _145 = false;
    }
    bool _158 = false;
    if (_145)
    {
        float3 _148 = clamp(_10_testInputs.xyz, float3(-1.0f, -2.0f, -2.0f), float3(1.0f, 2.0f, 0.5f));
        _158 = all(bool3(_148.x == expectedB.xyz.x, _148.y == expectedB.xyz.y, _148.z == expectedB.xyz.z));
    }
    else
    {
        _158 = false;
    }
    bool _169 = false;
    if (_158)
    {
        float4 _161 = clamp(_10_testInputs, clampLow, clampHigh);
        _169 = all(bool4(_161.x == expectedB.x, _161.y == expectedB.y, _161.z == expectedB.z, _161.w == expectedB.w));
    }
    else
    {
        _169 = false;
    }
    bool _175 = false;
    if (_169)
    {
        _175 = (-1.0f) == expectedB.x;
    }
    else
    {
        _175 = false;
    }
    bool _182 = false;
    if (_175)
    {
        _182 = all(bool2(float2(-1.0f, 0.0f).x == expectedB.xy.x, float2(-1.0f, 0.0f).y == expectedB.xy.y));
    }
    else
    {
        _182 = false;
    }
    bool _190 = false;
    if (_182)
    {
        _190 = all(bool3(float3(-1.0f, 0.0f, 0.5f).x == expectedB.xyz.x, float3(-1.0f, 0.0f, 0.5f).y == expectedB.xyz.y, float3(-1.0f, 0.0f, 0.5f).z == expectedB.xyz.z));
    }
    else
    {
        _190 = false;
    }
    bool _196 = false;
    if (_190)
    {
        _196 = all(bool4(float4(-1.0f, 0.0f, 0.5f, 2.25f).x == expectedB.x, float4(-1.0f, 0.0f, 0.5f, 2.25f).y == expectedB.y, float4(-1.0f, 0.0f, 0.5f, 2.25f).z == expectedB.z, float4(-1.0f, 0.0f, 0.5f, 2.25f).w == expectedB.w));
    }
    else
    {
        _196 = false;
    }
    float4 _197 = 0.0f.xxxx;
    if (_196)
    {
        _197 = _10_colorGreen;
    }
    else
    {
        _197 = _10_colorRed;
    }
    return _197;
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
