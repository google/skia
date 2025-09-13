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
    float4 expectedA = float4(0.0f, 0.0f, 1.0f, 1.0f);
    float4 expectedB = float4(1.0f, 1.0f, 0.0f, 0.0f);
    float4 expectedC = float4(0.0f, 1.0f, 1.0f, 1.0f);
    bool _56 = false;
    if (step(0.5f, _11_testInputs.x) == 0.0f)
    {
        float2 _47 = step(0.5f.xx, _11_testInputs.xy);
        _56 = all(bool2(_47.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, _47.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _56 = false;
    }
    bool _69 = false;
    if (_56)
    {
        float3 _59 = step(0.5f.xxx, _11_testInputs.xyz);
        _69 = all(bool3(_59.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, _59.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, _59.z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _69 = false;
    }
    bool _79 = false;
    if (_69)
    {
        float4 _72 = step(0.5f.xxxx, _11_testInputs);
        _79 = all(bool4(_72.x == float4(0.0f, 0.0f, 1.0f, 1.0f).x, _72.y == float4(0.0f, 0.0f, 1.0f, 1.0f).y, _72.z == float4(0.0f, 0.0f, 1.0f, 1.0f).z, _72.w == float4(0.0f, 0.0f, 1.0f, 1.0f).w));
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
        _89 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _89 = false;
    }
    bool _96 = false;
    if (_89)
    {
        _96 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(0.0f, 0.0f, 1.0f).y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(0.0f, 0.0f, 1.0f).z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
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
    bool _107 = false;
    if (_99)
    {
        _107 = step(_11_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _107 = false;
    }
    bool _118 = false;
    if (_107)
    {
        float2 _110 = step(_11_testInputs.xy, float2(0.0f, 1.0f));
        _118 = all(bool2(_110.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, _110.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _118 = false;
    }
    bool _129 = false;
    if (_118)
    {
        float3 _121 = step(_11_testInputs.xyz, float3(0.0f, 1.0f, 0.0f));
        _129 = all(bool3(_121.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, _121.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, _121.z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _129 = false;
    }
    bool _138 = false;
    if (_129)
    {
        float4 _132 = step(_11_testInputs, float4(0.0f, 1.0f, 0.0f, 1.0f));
        _138 = all(bool4(_132.x == float4(1.0f, 1.0f, 0.0f, 0.0f).x, _132.y == float4(1.0f, 1.0f, 0.0f, 0.0f).y, _132.z == float4(1.0f, 1.0f, 0.0f, 0.0f).z, _132.w == float4(1.0f, 1.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _138 = false;
    }
    bool _141 = false;
    if (_138)
    {
        _141 = true;
    }
    else
    {
        _141 = false;
    }
    bool _148 = false;
    if (_141)
    {
        _148 = all(bool2(1.0f.xx.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, 1.0f.xx.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _148 = false;
    }
    bool _155 = false;
    if (_148)
    {
        _155 = all(bool3(float3(1.0f, 1.0f, 0.0f).x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, float3(1.0f, 1.0f, 0.0f).y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, float3(1.0f, 1.0f, 0.0f).z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _155 = false;
    }
    bool _158 = false;
    if (_155)
    {
        _158 = true;
    }
    else
    {
        _158 = false;
    }
    bool _171 = false;
    if (_158)
    {
        _171 = step(_11_colorRed.x, _11_colorGreen.x) == 0.0f;
    }
    else
    {
        _171 = false;
    }
    bool _184 = false;
    if (_171)
    {
        float2 _174 = step(_11_colorRed.xy, _11_colorGreen.xy);
        _184 = all(bool2(_174.x == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.x, _174.y == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _184 = false;
    }
    bool _197 = false;
    if (_184)
    {
        float3 _187 = step(_11_colorRed.xyz, _11_colorGreen.xyz);
        _197 = all(bool3(_187.x == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.x, _187.y == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.y, _187.z == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _197 = false;
    }
    bool _207 = false;
    if (_197)
    {
        float4 _200 = step(_11_colorRed, _11_colorGreen);
        _207 = all(bool4(_200.x == float4(0.0f, 1.0f, 1.0f, 1.0f).x, _200.y == float4(0.0f, 1.0f, 1.0f, 1.0f).y, _200.z == float4(0.0f, 1.0f, 1.0f, 1.0f).z, _200.w == float4(0.0f, 1.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _207 = false;
    }
    bool _210 = false;
    if (_207)
    {
        _210 = true;
    }
    else
    {
        _210 = false;
    }
    bool _216 = false;
    if (_210)
    {
        _216 = all(bool2(float2(0.0f, 1.0f).x == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.x, float2(0.0f, 1.0f).y == float4(0.0f, 1.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _216 = false;
    }
    bool _223 = false;
    if (_216)
    {
        _223 = all(bool3(float3(0.0f, 1.0f, 1.0f).x == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.x, float3(0.0f, 1.0f, 1.0f).y == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.y, float3(0.0f, 1.0f, 1.0f).z == float4(0.0f, 1.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _223 = false;
    }
    bool _226 = false;
    if (_223)
    {
        _226 = true;
    }
    else
    {
        _226 = false;
    }
    float4 _227 = 0.0f.xxxx;
    if (_226)
    {
        _227 = _11_colorGreen;
    }
    else
    {
        _227 = _11_colorRed;
    }
    return _227;
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
