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
    float4 expectedA = float4(0.5f, 0.5f, 0.75f, 2.25f);
    float4 expectedB = float4(0.0f, 1.0f, 0.75f, 2.25f);
    bool _56 = false;
    if (max(_11_testInputs.x, 0.5f) == 0.5f)
    {
        float2 _47 = max(_11_testInputs.xy, 0.5f.xx);
        _56 = all(bool2(_47.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, _47.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _56 = false;
    }
    bool _69 = false;
    if (_56)
    {
        float3 _59 = max(_11_testInputs.xyz, 0.5f.xxx);
        _69 = all(bool3(_59.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, _59.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, _59.z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _69 = false;
    }
    bool _79 = false;
    if (_69)
    {
        float4 _72 = max(_11_testInputs, 0.5f.xxxx);
        _79 = all(bool4(_72.x == float4(0.5f, 0.5f, 0.75f, 2.25f).x, _72.y == float4(0.5f, 0.5f, 0.75f, 2.25f).y, _72.z == float4(0.5f, 0.5f, 0.75f, 2.25f).z, _72.w == float4(0.5f, 0.5f, 0.75f, 2.25f).w));
    }
    else
    {
        _79 = false;
    }
    bool _83 = false;
    if (_79)
    {
        _83 = true;
    }
    else
    {
        _83 = false;
    }
    bool _89 = false;
    if (_83)
    {
        _89 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool3(float3(0.5f, 0.5f, 0.75f).x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, float3(0.5f, 0.5f, 0.75f).y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, float3(0.5f, 0.5f, 0.75f).z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _96 = false;
    }
    bool _99 = false;
    if (_96)
    {
        _99 = true;
    }
    else
    {
        _99 = false;
    }
    bool _111 = false;
    if (_99)
    {
        _111 = max(_11_testInputs.x, _11_colorGreen.x) == 0.0f;
    }
    else
    {
        _111 = false;
    }
    bool _124 = false;
    if (_111)
    {
        float2 _114 = max(_11_testInputs.xy, _11_colorGreen.xy);
        _124 = all(bool2(_114.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, _114.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _124 = false;
    }
    bool _137 = false;
    if (_124)
    {
        float3 _127 = max(_11_testInputs.xyz, _11_colorGreen.xyz);
        _137 = all(bool3(_127.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, _127.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, _127.z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _137 = false;
    }
    bool _147 = false;
    if (_137)
    {
        float4 _140 = max(_11_testInputs, _11_colorGreen);
        _147 = all(bool4(_140.x == float4(0.0f, 1.0f, 0.75f, 2.25f).x, _140.y == float4(0.0f, 1.0f, 0.75f, 2.25f).y, _140.z == float4(0.0f, 1.0f, 0.75f, 2.25f).z, _140.w == float4(0.0f, 1.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _147 = false;
    }
    bool _150 = false;
    if (_147)
    {
        _150 = true;
    }
    else
    {
        _150 = false;
    }
    bool _157 = false;
    if (_150)
    {
        _157 = all(bool2(float2(0.0f, 1.0f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, float2(0.0f, 1.0f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool3(float3(0.0f, 1.0f, 0.75f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, float3(0.0f, 1.0f, 0.75f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, float3(0.0f, 1.0f, 0.75f).z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _164 = false;
    }
    bool _167 = false;
    if (_164)
    {
        _167 = true;
    }
    else
    {
        _167 = false;
    }
    float4 _168 = 0.0f.xxxx;
    if (_167)
    {
        _168 = _11_colorGreen;
    }
    else
    {
        _168 = _11_colorRed;
    }
    return _168;
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
