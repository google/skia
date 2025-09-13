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
    float4 expectedA = float4(-1.25f, 0.0f, 0.5f, 0.5f);
    float4 expectedB = float4(-1.25f, 0.0f, 0.0f, 1.0f);
    bool _55 = false;
    if (min(_11_testInputs.x, 0.5f) == (-1.25f))
    {
        float2 _46 = min(_11_testInputs.xy, 0.5f.xx);
        _55 = all(bool2(_46.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, _46.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _68 = false;
    if (_55)
    {
        float3 _58 = min(_11_testInputs.xyz, 0.5f.xxx);
        _68 = all(bool3(_58.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, _58.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, _58.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _78 = false;
    if (_68)
    {
        float4 _71 = min(_11_testInputs, 0.5f.xxxx);
        _78 = all(bool4(_71.x == float4(-1.25f, 0.0f, 0.5f, 0.5f).x, _71.y == float4(-1.25f, 0.0f, 0.5f, 0.5f).y, _71.z == float4(-1.25f, 0.0f, 0.5f, 0.5f).z, _71.w == float4(-1.25f, 0.0f, 0.5f, 0.5f).w));
    }
    else
    {
        _78 = false;
    }
    bool _82 = false;
    if (_78)
    {
        _82 = true;
    }
    else
    {
        _82 = false;
    }
    bool _89 = false;
    if (_82)
    {
        _89 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool3(float3(-1.25f, 0.0f, 0.5f).x == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.x, float3(-1.25f, 0.0f, 0.5f).y == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.y, float3(-1.25f, 0.0f, 0.5f).z == float4(-1.25f, 0.0f, 0.5f, 0.5f).xyz.z));
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
        _111 = min(_11_testInputs.x, _11_colorGreen.x) == (-1.25f);
    }
    else
    {
        _111 = false;
    }
    bool _124 = false;
    if (_111)
    {
        float2 _114 = min(_11_testInputs.xy, _11_colorGreen.xy);
        _124 = all(bool2(_114.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, _114.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _124 = false;
    }
    bool _137 = false;
    if (_124)
    {
        float3 _127 = min(_11_testInputs.xyz, _11_colorGreen.xyz);
        _137 = all(bool3(_127.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, _127.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, _127.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _137 = false;
    }
    bool _147 = false;
    if (_137)
    {
        float4 _140 = min(_11_testInputs, _11_colorGreen);
        _147 = all(bool4(_140.x == float4(-1.25f, 0.0f, 0.0f, 1.0f).x, _140.y == float4(-1.25f, 0.0f, 0.0f, 1.0f).y, _140.z == float4(-1.25f, 0.0f, 0.0f, 1.0f).z, _140.w == float4(-1.25f, 0.0f, 0.0f, 1.0f).w));
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
    bool _156 = false;
    if (_150)
    {
        _156 = all(bool2(float2(-1.25f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.x, float2(-1.25f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xy.y));
    }
    else
    {
        _156 = false;
    }
    bool _163 = false;
    if (_156)
    {
        _163 = all(bool3(float3(-1.25f, 0.0f, 0.0f).x == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.x, float3(-1.25f, 0.0f, 0.0f).y == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.y, float3(-1.25f, 0.0f, 0.0f).z == float4(-1.25f, 0.0f, 0.0f, 1.0f).xyz.z));
    }
    else
    {
        _163 = false;
    }
    bool _166 = false;
    if (_163)
    {
        _166 = true;
    }
    else
    {
        _166 = false;
    }
    float4 _167 = 0.0f.xxxx;
    if (_166)
    {
        _167 = _11_colorGreen;
    }
    else
    {
        _167 = _11_colorRed;
    }
    return _167;
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
