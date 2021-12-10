cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

float4 main(float2 _24)
{
    float4 expectedA = float4(0.75f, 0.0f, 0.75f, 0.25f);
    float4 expectedB = float4(0.25f, 0.0f, 0.75f, 1.0f);
    bool _57 = false;
    if (mod(_10_testInputs.x, 1.0f) == expectedA.x)
    {
        float2 _47 = mod(_10_testInputs.xy, 1.0f.xx);
        _57 = all(bool2(_47.x == expectedA.xy.x, _47.y == expectedA.xy.y));
    }
    else
    {
        _57 = false;
    }
    bool _71 = false;
    if (_57)
    {
        float3 _60 = mod(_10_testInputs.xyz, 1.0f.xxx);
        _71 = all(bool3(_60.x == expectedA.xyz.x, _60.y == expectedA.xyz.y, _60.z == expectedA.xyz.z));
    }
    else
    {
        _71 = false;
    }
    bool _82 = false;
    if (_71)
    {
        float4 _74 = mod(_10_testInputs, 1.0f.xxxx);
        _82 = all(bool4(_74.x == expectedA.x, _74.y == expectedA.y, _74.z == expectedA.z, _74.w == expectedA.w));
    }
    else
    {
        _82 = false;
    }
    bool _88 = false;
    if (_82)
    {
        _88 = 0.75f == expectedA.x;
    }
    else
    {
        _88 = false;
    }
    bool _96 = false;
    if (_88)
    {
        _96 = all(bool2(float2(0.75f, 0.0f).x == expectedA.xy.x, float2(0.75f, 0.0f).y == expectedA.xy.y));
    }
    else
    {
        _96 = false;
    }
    bool _104 = false;
    if (_96)
    {
        _104 = all(bool3(float3(0.75f, 0.0f, 0.75f).x == expectedA.xyz.x, float3(0.75f, 0.0f, 0.75f).y == expectedA.xyz.y, float3(0.75f, 0.0f, 0.75f).z == expectedA.xyz.z));
    }
    else
    {
        _104 = false;
    }
    bool _110 = false;
    if (_104)
    {
        _110 = all(bool4(float4(0.75f, 0.0f, 0.75f, 0.25f).x == expectedA.x, float4(0.75f, 0.0f, 0.75f, 0.25f).y == expectedA.y, float4(0.75f, 0.0f, 0.75f, 0.25f).z == expectedA.z, float4(0.75f, 0.0f, 0.75f, 0.25f).w == expectedA.w));
    }
    else
    {
        _110 = false;
    }
    bool _124 = false;
    if (_110)
    {
        _124 = mod(_10_testInputs.x, _10_colorWhite.x) == expectedA.x;
    }
    else
    {
        _124 = false;
    }
    bool _138 = false;
    if (_124)
    {
        float2 _127 = mod(_10_testInputs.xy, _10_colorWhite.xy);
        _138 = all(bool2(_127.x == expectedA.xy.x, _127.y == expectedA.xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _152 = false;
    if (_138)
    {
        float3 _141 = mod(_10_testInputs.xyz, _10_colorWhite.xyz);
        _152 = all(bool3(_141.x == expectedA.xyz.x, _141.y == expectedA.xyz.y, _141.z == expectedA.xyz.z));
    }
    else
    {
        _152 = false;
    }
    bool _163 = false;
    if (_152)
    {
        float4 _155 = mod(_10_testInputs, _10_colorWhite);
        _163 = all(bool4(_155.x == expectedA.x, _155.y == expectedA.y, _155.z == expectedA.z, _155.w == expectedA.w));
    }
    else
    {
        _163 = false;
    }
    bool _169 = false;
    if (_163)
    {
        _169 = 0.25f == expectedB.x;
    }
    else
    {
        _169 = false;
    }
    bool _177 = false;
    if (_169)
    {
        _177 = all(bool2(float2(0.25f, 0.0f).x == expectedB.xy.x, float2(0.25f, 0.0f).y == expectedB.xy.y));
    }
    else
    {
        _177 = false;
    }
    bool _185 = false;
    if (_177)
    {
        _185 = all(bool3(float3(0.25f, 0.0f, 0.75f).x == expectedB.xyz.x, float3(0.25f, 0.0f, 0.75f).y == expectedB.xyz.y, float3(0.25f, 0.0f, 0.75f).z == expectedB.xyz.z));
    }
    else
    {
        _185 = false;
    }
    bool _191 = false;
    if (_185)
    {
        _191 = all(bool4(float4(0.25f, 0.0f, 0.75f, 1.0f).x == expectedB.x, float4(0.25f, 0.0f, 0.75f, 1.0f).y == expectedB.y, float4(0.25f, 0.0f, 0.75f, 1.0f).z == expectedB.z, float4(0.25f, 0.0f, 0.75f, 1.0f).w == expectedB.w));
    }
    else
    {
        _191 = false;
    }
    float4 _192 = 0.0f.xxxx;
    if (_191)
    {
        _192 = _10_colorGreen;
    }
    else
    {
        _192 = _10_colorRed;
    }
    return _192;
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
