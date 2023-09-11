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
    float4 expectedA = float4(0.0f, 0.0f, 1.0f, 1.0f);
    float4 expectedB = float4(1.0f, 1.0f, 0.0f, 0.0f);
    float4 expectedC = float4(0.0f, 1.0f, 1.0f, 1.0f);
    bool _55 = false;
    if (step(0.5f, _10_testInputs.x) == 0.0f)
    {
        float2 _46 = step(0.5f.xx, _10_testInputs.xy);
        _55 = all(bool2(_46.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, _46.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _55 = false;
    }
    bool _68 = false;
    if (_55)
    {
        float3 _58 = step(0.5f.xxx, _10_testInputs.xyz);
        _68 = all(bool3(_58.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, _58.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, _58.z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _78 = false;
    if (_68)
    {
        float4 _71 = step(0.5f.xxxx, _10_testInputs);
        _78 = all(bool4(_71.x == float4(0.0f, 0.0f, 1.0f, 1.0f).x, _71.y == float4(0.0f, 0.0f, 1.0f, 1.0f).y, _71.z == float4(0.0f, 0.0f, 1.0f, 1.0f).z, _71.w == float4(0.0f, 0.0f, 1.0f, 1.0f).w));
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
        _88 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _88 = false;
    }
    bool _95 = false;
    if (_88)
    {
        _95 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(0.0f, 0.0f, 1.0f).y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(0.0f, 0.0f, 1.0f).z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
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
    bool _106 = false;
    if (_98)
    {
        _106 = step(_10_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _106 = false;
    }
    bool _117 = false;
    if (_106)
    {
        float2 _109 = step(_10_testInputs.xy, float2(0.0f, 1.0f));
        _117 = all(bool2(_109.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, _109.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _117 = false;
    }
    bool _128 = false;
    if (_117)
    {
        float3 _120 = step(_10_testInputs.xyz, float3(0.0f, 1.0f, 0.0f));
        _128 = all(bool3(_120.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, _120.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, _120.z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _128 = false;
    }
    bool _137 = false;
    if (_128)
    {
        float4 _131 = step(_10_testInputs, float4(0.0f, 1.0f, 0.0f, 1.0f));
        _137 = all(bool4(_131.x == float4(1.0f, 1.0f, 0.0f, 0.0f).x, _131.y == float4(1.0f, 1.0f, 0.0f, 0.0f).y, _131.z == float4(1.0f, 1.0f, 0.0f, 0.0f).z, _131.w == float4(1.0f, 1.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _137 = false;
    }
    bool _140 = false;
    if (_137)
    {
        _140 = true;
    }
    else
    {
        _140 = false;
    }
    bool _147 = false;
    if (_140)
    {
        _147 = all(bool2(1.0f.xx.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, 1.0f.xx.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _147 = false;
    }
    bool _154 = false;
    if (_147)
    {
        _154 = all(bool3(float3(1.0f, 1.0f, 0.0f).x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, float3(1.0f, 1.0f, 0.0f).y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, float3(1.0f, 1.0f, 0.0f).z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _154 = false;
    }
    bool _157 = false;
    if (_154)
    {
        _157 = true;
    }
    else
    {
        _157 = false;
    }
    bool _170 = false;
    if (_157)
    {
        _170 = step(_10_colorRed.x, _10_colorGreen.x) == 0.0f;
    }
    else
    {
        _170 = false;
    }
    bool _183 = false;
    if (_170)
    {
        float2 _173 = step(_10_colorRed.xy, _10_colorGreen.xy);
        _183 = all(bool2(_173.x == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.x, _173.y == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _183 = false;
    }
    bool _196 = false;
    if (_183)
    {
        float3 _186 = step(_10_colorRed.xyz, _10_colorGreen.xyz);
        _196 = all(bool3(_186.x == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.x, _186.y == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.y, _186.z == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _196 = false;
    }
    bool _206 = false;
    if (_196)
    {
        float4 _199 = step(_10_colorRed, _10_colorGreen);
        _206 = all(bool4(_199.x == float4(0.0f, 1.0f, 1.0f, 1.0f).x, _199.y == float4(0.0f, 1.0f, 1.0f, 1.0f).y, _199.z == float4(0.0f, 1.0f, 1.0f, 1.0f).z, _199.w == float4(0.0f, 1.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _206 = false;
    }
    bool _209 = false;
    if (_206)
    {
        _209 = true;
    }
    else
    {
        _209 = false;
    }
    bool _215 = false;
    if (_209)
    {
        _215 = all(bool2(float2(0.0f, 1.0f).x == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.x, float2(0.0f, 1.0f).y == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _215 = false;
    }
    bool _222 = false;
    if (_215)
    {
        _222 = all(bool3(float3(0.0f, 1.0f, 1.0f).x == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.x, float3(0.0f, 1.0f, 1.0f).y == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.y, float3(0.0f, 1.0f, 1.0f).z == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _222 = false;
    }
    bool _225 = false;
    if (_222)
    {
        _225 = true;
    }
    else
    {
        _225 = false;
    }
    float4 _226 = 0.0f.xxxx;
    if (_225)
    {
        _226 = _10_colorGreen;
    }
    else
    {
        _226 = _10_colorRed;
    }
    return _226;
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
