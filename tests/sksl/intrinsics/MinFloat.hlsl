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
    float4 expectedA = float4(-1.25f, 0.0f, 0.5f, 0.5f);
    float4 expectedB = float4(-1.25f, 0.0f, 0.0f, 1.0f);
    bool _57 = false;
    if (min(_10_testInputs.x, 0.5f) == expectedA.x)
    {
        float2 _47 = min(_10_testInputs.xy, 0.5f.xx);
        _57 = all(bool2(_47.x == expectedA.xy.x, _47.y == expectedA.xy.y));
    }
    else
    {
        _57 = false;
    }
    bool _71 = false;
    if (_57)
    {
        float3 _60 = min(_10_testInputs.xyz, 0.5f.xxx);
        _71 = all(bool3(_60.x == expectedA.xyz.x, _60.y == expectedA.xyz.y, _60.z == expectedA.xyz.z));
    }
    else
    {
        _71 = false;
    }
    bool _82 = false;
    if (_71)
    {
        float4 _74 = min(_10_testInputs, 0.5f.xxxx);
        _82 = all(bool4(_74.x == expectedA.x, _74.y == expectedA.y, _74.z == expectedA.z, _74.w == expectedA.w));
    }
    else
    {
        _82 = false;
    }
    bool _88 = false;
    if (_82)
    {
        _88 = (-1.25f) == expectedA.x;
    }
    else
    {
        _88 = false;
    }
    bool _96 = false;
    if (_88)
    {
        _96 = all(bool2(float2(-1.25f, 0.0f).x == expectedA.xy.x, float2(-1.25f, 0.0f).y == expectedA.xy.y));
    }
    else
    {
        _96 = false;
    }
    bool _104 = false;
    if (_96)
    {
        _104 = all(bool3(float3(-1.25f, 0.0f, 0.5f).x == expectedA.xyz.x, float3(-1.25f, 0.0f, 0.5f).y == expectedA.xyz.y, float3(-1.25f, 0.0f, 0.5f).z == expectedA.xyz.z));
    }
    else
    {
        _104 = false;
    }
    bool _110 = false;
    if (_104)
    {
        _110 = all(bool4(float4(-1.25f, 0.0f, 0.5f, 0.5f).x == expectedA.x, float4(-1.25f, 0.0f, 0.5f, 0.5f).y == expectedA.y, float4(-1.25f, 0.0f, 0.5f, 0.5f).z == expectedA.z, float4(-1.25f, 0.0f, 0.5f, 0.5f).w == expectedA.w));
    }
    else
    {
        _110 = false;
    }
    bool _124 = false;
    if (_110)
    {
        _124 = min(_10_testInputs.x, _10_colorGreen.x) == expectedB.x;
    }
    else
    {
        _124 = false;
    }
    bool _138 = false;
    if (_124)
    {
        float2 _127 = min(_10_testInputs.xy, _10_colorGreen.xy);
        _138 = all(bool2(_127.x == expectedB.xy.x, _127.y == expectedB.xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _152 = false;
    if (_138)
    {
        float3 _141 = min(_10_testInputs.xyz, _10_colorGreen.xyz);
        _152 = all(bool3(_141.x == expectedB.xyz.x, _141.y == expectedB.xyz.y, _141.z == expectedB.xyz.z));
    }
    else
    {
        _152 = false;
    }
    bool _163 = false;
    if (_152)
    {
        float4 _155 = min(_10_testInputs, _10_colorGreen);
        _163 = all(bool4(_155.x == expectedB.x, _155.y == expectedB.y, _155.z == expectedB.z, _155.w == expectedB.w));
    }
    else
    {
        _163 = false;
    }
    bool _169 = false;
    if (_163)
    {
        _169 = (-1.25f) == expectedB.x;
    }
    else
    {
        _169 = false;
    }
    bool _176 = false;
    if (_169)
    {
        _176 = all(bool2(float2(-1.25f, 0.0f).x == expectedB.xy.x, float2(-1.25f, 0.0f).y == expectedB.xy.y));
    }
    else
    {
        _176 = false;
    }
    bool _184 = false;
    if (_176)
    {
        _184 = all(bool3(float3(-1.25f, 0.0f, 0.0f).x == expectedB.xyz.x, float3(-1.25f, 0.0f, 0.0f).y == expectedB.xyz.y, float3(-1.25f, 0.0f, 0.0f).z == expectedB.xyz.z));
    }
    else
    {
        _184 = false;
    }
    bool _190 = false;
    if (_184)
    {
        _190 = all(bool4(float4(-1.25f, 0.0f, 0.0f, 1.0f).x == expectedB.x, float4(-1.25f, 0.0f, 0.0f, 1.0f).y == expectedB.y, float4(-1.25f, 0.0f, 0.0f, 1.0f).z == expectedB.z, float4(-1.25f, 0.0f, 0.0f, 1.0f).w == expectedB.w));
    }
    else
    {
        _190 = false;
    }
    float4 _191 = 0.0f.xxxx;
    if (_190)
    {
        _191 = _10_colorGreen;
    }
    else
    {
        _191 = _10_colorRed;
    }
    return _191;
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
