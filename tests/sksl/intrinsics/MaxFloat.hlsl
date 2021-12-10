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
    float4 expectedA = float4(0.5f, 0.5f, 0.75f, 2.25f);
    float4 expectedB = float4(0.0f, 1.0f, 0.75f, 2.25f);
    bool _55 = false;
    if (max(_10_testInputs.x, 0.5f) == 0.5f)
    {
        float2 _46 = max(_10_testInputs.xy, 0.5f.xx);
        _55 = all(bool2(_46.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, _46.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _68 = false;
    if (_55)
    {
        float3 _58 = max(_10_testInputs.xyz, 0.5f.xxx);
        _68 = all(bool3(_58.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, _58.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, _58.z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _78 = false;
    if (_68)
    {
        float4 _71 = max(_10_testInputs, 0.5f.xxxx);
        _78 = all(bool4(_71.x == float4(0.5f, 0.5f, 0.75f, 2.25f).x, _71.y == float4(0.5f, 0.5f, 0.75f, 2.25f).y, _71.z == float4(0.5f, 0.5f, 0.75f, 2.25f).z, _71.w == float4(0.5f, 0.5f, 0.75f, 2.25f).w));
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
    bool _88 = false;
    if (_82)
    {
        _88 = all(bool2(0.5f.xx.x == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.x, 0.5f.xx.y == float4(0.5f, 0.5f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _88 = false;
    }
    bool _95 = false;
    if (_88)
    {
        _95 = all(bool3(float3(0.5f, 0.5f, 0.75f).x == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.x, float3(0.5f, 0.5f, 0.75f).y == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.y, float3(0.5f, 0.5f, 0.75f).z == float4(0.5f, 0.5f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _95 = false;
    }
    bool _98 = false;
    if (_95)
    {
        _98 = true;
    }
    else
    {
        _98 = false;
    }
    bool _110 = false;
    if (_98)
    {
        _110 = max(_10_testInputs.x, _10_colorGreen.x) == 0.0f;
    }
    else
    {
        _110 = false;
    }
    bool _123 = false;
    if (_110)
    {
        float2 _113 = max(_10_testInputs.xy, _10_colorGreen.xy);
        _123 = all(bool2(_113.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, _113.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _123 = false;
    }
    bool _136 = false;
    if (_123)
    {
        float3 _126 = max(_10_testInputs.xyz, _10_colorGreen.xyz);
        _136 = all(bool3(_126.x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, _126.y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, _126.z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
    }
    else
    {
        _136 = false;
    }
    bool _146 = false;
    if (_136)
    {
        float4 _139 = max(_10_testInputs, _10_colorGreen);
        _146 = all(bool4(_139.x == float4(0.0f, 1.0f, 0.75f, 2.25f).x, _139.y == float4(0.0f, 1.0f, 0.75f, 2.25f).y, _139.z == float4(0.0f, 1.0f, 0.75f, 2.25f).z, _139.w == float4(0.0f, 1.0f, 0.75f, 2.25f).w));
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
    bool _156 = false;
    if (_149)
    {
        _156 = all(bool2(float2(0.0f, 1.0f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.x, float2(0.0f, 1.0f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xy.y));
    }
    else
    {
        _156 = false;
    }
    bool _163 = false;
    if (_156)
    {
        _163 = all(bool3(float3(0.0f, 1.0f, 0.75f).x == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.x, float3(0.0f, 1.0f, 0.75f).y == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.y, float3(0.0f, 1.0f, 0.75f).z == float4(0.0f, 1.0f, 0.75f, 2.25f).xyz.z));
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
        _167 = _10_colorGreen;
    }
    else
    {
        _167 = _10_colorRed;
    }
    return _167;
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
