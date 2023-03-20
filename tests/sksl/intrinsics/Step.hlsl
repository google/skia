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
    bool _53 = false;
    if (step(0.5f, _10_testInputs.x) == 0.0f)
    {
        float2 _44 = step(0.5f.xx, _10_testInputs.xy);
        _53 = all(bool2(_44.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, _44.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _53 = false;
    }
    bool _66 = false;
    if (_53)
    {
        float3 _56 = step(0.5f.xxx, _10_testInputs.xyz);
        _66 = all(bool3(_56.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, _56.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, _56.z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _76 = false;
    if (_66)
    {
        float4 _69 = step(0.5f.xxxx, _10_testInputs);
        _76 = all(bool4(_69.x == float4(0.0f, 0.0f, 1.0f, 1.0f).x, _69.y == float4(0.0f, 0.0f, 1.0f, 1.0f).y, _69.z == float4(0.0f, 0.0f, 1.0f, 1.0f).z, _69.w == float4(0.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _76 = false;
    }
    bool _80 = false;
    if (_76)
    {
        _80 = true;
    }
    else
    {
        _80 = false;
    }
    bool _86 = false;
    if (_80)
    {
        _86 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _86 = false;
    }
    bool _93 = false;
    if (_86)
    {
        _93 = all(bool3(float3(0.0f, 0.0f, 1.0f).x == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(0.0f, 0.0f, 1.0f).y == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(0.0f, 0.0f, 1.0f).z == float4(0.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _93 = false;
    }
    bool _96 = false;
    if (_93)
    {
        _96 = true;
    }
    else
    {
        _96 = false;
    }
    bool _104 = false;
    if (_96)
    {
        _104 = step(_10_testInputs.x, 0.0f) == 1.0f;
    }
    else
    {
        _104 = false;
    }
    bool _115 = false;
    if (_104)
    {
        float2 _107 = step(_10_testInputs.xy, float2(0.0f, 1.0f));
        _115 = all(bool2(_107.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, _107.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _115 = false;
    }
    bool _126 = false;
    if (_115)
    {
        float3 _118 = step(_10_testInputs.xyz, float3(0.0f, 1.0f, 0.0f));
        _126 = all(bool3(_118.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, _118.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, _118.z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _126 = false;
    }
    bool _135 = false;
    if (_126)
    {
        float4 _129 = step(_10_testInputs, float4(0.0f, 1.0f, 0.0f, 1.0f));
        _135 = all(bool4(_129.x == float4(1.0f, 1.0f, 0.0f, 0.0f).x, _129.y == float4(1.0f, 1.0f, 0.0f, 0.0f).y, _129.z == float4(1.0f, 1.0f, 0.0f, 0.0f).z, _129.w == float4(1.0f, 1.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _135 = false;
    }
    bool _138 = false;
    if (_135)
    {
        _138 = true;
    }
    else
    {
        _138 = false;
    }
    bool _145 = false;
    if (_138)
    {
        _145 = all(bool2(1.0f.xx.x == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.x, 1.0f.xx.y == float4(1.0f, 1.0f, 0.0f, 0.0f).xy.y));
    }
    else
    {
        _145 = false;
    }
    bool _152 = false;
    if (_145)
    {
        _152 = all(bool3(float3(1.0f, 1.0f, 0.0f).x == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.x, float3(1.0f, 1.0f, 0.0f).y == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.y, float3(1.0f, 1.0f, 0.0f).z == float4(1.0f, 1.0f, 0.0f, 0.0f).xyz.z));
    }
    else
    {
        _152 = false;
    }
    bool _155 = false;
    if (_152)
    {
        _155 = true;
    }
    else
    {
        _155 = false;
    }
    float4 _156 = 0.0f.xxxx;
    if (_155)
    {
        _156 = _10_colorGreen;
    }
    else
    {
        _156 = _10_colorRed;
    }
    return _156;
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
