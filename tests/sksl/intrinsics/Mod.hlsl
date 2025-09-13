cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
    float4 _11_colorWhite : packoffset(c3);
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

float4 main(float2 _25)
{
    float4 expectedA = float4(0.75f, 0.0f, 0.75f, 0.25f);
    float4 expectedB = float4(0.25f, 0.0f, 0.75f, 1.0f);
    bool _55 = false;
    if (mod(_11_testInputs.x, 1.0f) == 0.75f)
    {
        float2 _46 = mod(_11_testInputs.xy, 1.0f.xx);
        _55 = all(bool2(_46.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _46.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _68 = false;
    if (_55)
    {
        float3 _58 = mod(_11_testInputs.xyz, 1.0f.xxx);
        _68 = all(bool3(_58.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _58.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _58.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _78 = false;
    if (_68)
    {
        float4 _71 = mod(_11_testInputs, 1.0f.xxxx);
        _78 = all(bool4(_71.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _71.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _71.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _71.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
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
        _89 = all(bool2(float2(0.75f, 0.0f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, float2(0.75f, 0.0f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool3(float3(0.75f, 0.0f, 0.75f).x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, float3(0.75f, 0.0f, 0.75f).y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, float3(0.75f, 0.0f, 0.75f).z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
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
        _111 = mod(_11_testInputs.x, _11_colorWhite.x) == 0.75f;
    }
    else
    {
        _111 = false;
    }
    bool _124 = false;
    if (_111)
    {
        float2 _114 = mod(_11_testInputs.xy, _11_colorWhite.xy);
        _124 = all(bool2(_114.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.x, _114.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xy.y));
    }
    else
    {
        _124 = false;
    }
    bool _137 = false;
    if (_124)
    {
        float3 _127 = mod(_11_testInputs.xyz, _11_colorWhite.xyz);
        _137 = all(bool3(_127.x == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.x, _127.y == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.y, _127.z == float4(0.75f, 0.0f, 0.75f, 0.25f).xyz.z));
    }
    else
    {
        _137 = false;
    }
    bool _147 = false;
    if (_137)
    {
        float4 _140 = mod(_11_testInputs, _11_colorWhite);
        _147 = all(bool4(_140.x == float4(0.75f, 0.0f, 0.75f, 0.25f).x, _140.y == float4(0.75f, 0.0f, 0.75f, 0.25f).y, _140.z == float4(0.75f, 0.0f, 0.75f, 0.25f).z, _140.w == float4(0.75f, 0.0f, 0.75f, 0.25f).w));
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
        _157 = all(bool2(float2(0.25f, 0.0f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.x, float2(0.25f, 0.0f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool3(float3(0.25f, 0.0f, 0.75f).x == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.x, float3(0.25f, 0.0f, 0.75f).y == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.y, float3(0.25f, 0.0f, 0.75f).z == float4(0.25f, 0.0f, 0.75f, 1.0f).xyz.z));
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
