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
    float4 expectedB = float4(-1.0f, 0.0f, 0.5f, 2.25f);
    bool _57 = false;
    if (clamp(_10_testInputs.x, -1.0f, 1.0f) == (-1.0f))
    {
        float2 _47 = clamp(_10_testInputs.xy, (-1.0f).xx, 1.0f.xx);
        _57 = all(bool2(_47.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, _47.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _57 = false;
    }
    bool _71 = false;
    if (_57)
    {
        float3 _60 = clamp(_10_testInputs.xyz, (-1.0f).xxx, 1.0f.xxx);
        _71 = all(bool3(_60.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, _60.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, _60.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _71 = false;
    }
    bool _82 = false;
    if (_71)
    {
        float4 _74 = clamp(_10_testInputs, (-1.0f).xxxx, 1.0f.xxxx);
        _82 = all(bool4(_74.x == float4(-1.0f, 0.0f, 0.75f, 1.0f).x, _74.y == float4(-1.0f, 0.0f, 0.75f, 1.0f).y, _74.z == float4(-1.0f, 0.0f, 0.75f, 1.0f).z, _74.w == float4(-1.0f, 0.0f, 0.75f, 1.0f).w));
    }
    else
    {
        _82 = false;
    }
    bool _90 = false;
    if (_82)
    {
        _90 = clamp(_10_testInputs.x, -1.0f, 1.0f) == (-1.0f);
    }
    else
    {
        _90 = false;
    }
    bool _104 = false;
    if (_90)
    {
        float2 _93 = clamp(_10_testInputs.xy, float2(-1.0f, -2.0f), float2(1.0f, 2.0f));
        _104 = all(bool2(_93.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, _93.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _104 = false;
    }
    bool _116 = false;
    if (_104)
    {
        float3 _107 = clamp(_10_testInputs.xyz, float3(-1.0f, -2.0f, -2.0f), float3(1.0f, 2.0f, 0.5f));
        _116 = all(bool3(_107.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, _107.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, _107.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
    }
    else
    {
        _116 = false;
    }
    bool _127 = false;
    if (_116)
    {
        float4 _119 = clamp(_10_testInputs, float4(-1.0f, -2.0f, -2.0f, 1.0f), float4(1.0f, 2.0f, 0.5f, 3.0f));
        _127 = all(bool4(_119.x == float4(-1.0f, 0.0f, 0.5f, 2.25f).x, _119.y == float4(-1.0f, 0.0f, 0.5f, 2.25f).y, _119.z == float4(-1.0f, 0.0f, 0.5f, 2.25f).z, _119.w == float4(-1.0f, 0.0f, 0.5f, 2.25f).w));
    }
    else
    {
        _127 = false;
    }
    bool _131 = false;
    if (_127)
    {
        _131 = true;
    }
    else
    {
        _131 = false;
    }
    bool _138 = false;
    if (_131)
    {
        _138 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _145 = false;
    if (_138)
    {
        _145 = all(bool3(float3(-1.0f, 0.0f, 0.75f).x == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.x, float3(-1.0f, 0.0f, 0.75f).y == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.y, float3(-1.0f, 0.0f, 0.75f).z == float4(-1.0f, 0.0f, 0.75f, 1.0f).xyz.z));
    }
    else
    {
        _145 = false;
    }
    bool _148 = false;
    if (_145)
    {
        _148 = true;
    }
    else
    {
        _148 = false;
    }
    bool _151 = false;
    if (_148)
    {
        _151 = true;
    }
    else
    {
        _151 = false;
    }
    bool _157 = false;
    if (_151)
    {
        _157 = all(bool2(float2(-1.0f, 0.0f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.x, float2(-1.0f, 0.0f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xy.y));
    }
    else
    {
        _157 = false;
    }
    bool _164 = false;
    if (_157)
    {
        _164 = all(bool3(float3(-1.0f, 0.0f, 0.5f).x == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.x, float3(-1.0f, 0.0f, 0.5f).y == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.y, float3(-1.0f, 0.0f, 0.5f).z == float4(-1.0f, 0.0f, 0.5f, 2.25f).xyz.z));
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
        _168 = _10_colorGreen;
    }
    else
    {
        _168 = _10_colorRed;
    }
    return _168;
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
