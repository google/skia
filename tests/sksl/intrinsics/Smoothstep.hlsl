cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 expectedA = float4(0.0f, 0.0f, 0.84375f, 1.0f);
    float4 expectedB = float4(1.0f, 0.0f, 1.0f, 1.0f);
    bool _39 = false;
    if (true)
    {
        _39 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _39 = false;
    }
    bool _48 = false;
    if (_39)
    {
        _48 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _48 = false;
    }
    bool _51 = false;
    if (_48)
    {
        _51 = true;
    }
    else
    {
        _51 = false;
    }
    bool _54 = false;
    if (_51)
    {
        _54 = true;
    }
    else
    {
        _54 = false;
    }
    bool _60 = false;
    if (_54)
    {
        _60 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _60 = false;
    }
    bool _66 = false;
    if (_60)
    {
        _66 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _66 = false;
    }
    bool _69 = false;
    if (_66)
    {
        _69 = true;
    }
    else
    {
        _69 = false;
    }
    bool _85 = false;
    if (_69)
    {
        _85 = smoothstep(_7_colorRed.y, _7_colorGreen.y, -1.25f) == 0.0f;
    }
    else
    {
        _85 = false;
    }
    bool _101 = false;
    if (_85)
    {
        float2 _88 = smoothstep(_7_colorRed.y.xx, _7_colorGreen.y.xx, float2(-1.25f, 0.0f));
        _101 = all(bool2(_88.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, _88.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _101 = false;
    }
    bool _118 = false;
    if (_101)
    {
        float3 _104 = smoothstep(_7_colorRed.y.xxx, _7_colorGreen.y.xxx, float3(-1.25f, 0.0f, 0.75f));
        _118 = all(bool3(_104.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, _104.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, _104.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _118 = false;
    }
    bool _135 = false;
    if (_118)
    {
        float4 _121 = smoothstep(_7_colorRed.y.xxxx, _7_colorGreen.y.xxxx, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _135 = all(bool4(_121.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).x, _121.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).y, _121.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).z, _121.w == float4(0.0f, 0.0f, 0.84375f, 1.0f).w));
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
        _145 = all(bool2(float2(1.0f, 0.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, float2(1.0f, 0.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _145 = false;
    }
    bool _152 = false;
    if (_145)
    {
        _152 = all(bool3(float3(1.0f, 0.0f, 1.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(1.0f, 0.0f, 1.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(1.0f, 0.0f, 1.0f).z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
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
    bool _166 = false;
    if (_155)
    {
        _166 = smoothstep(_7_colorRed.x, _7_colorGreen.x, -1.25f) == 1.0f;
    }
    else
    {
        _166 = false;
    }
    bool _179 = false;
    if (_166)
    {
        float2 _169 = smoothstep(_7_colorRed.xy, _7_colorGreen.xy, float2(-1.25f, 0.0f));
        _179 = all(bool2(_169.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, _169.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _179 = false;
    }
    bool _192 = false;
    if (_179)
    {
        float3 _182 = smoothstep(_7_colorRed.xyz, _7_colorGreen.xyz, float3(-1.25f, 0.0f, 0.75f));
        _192 = all(bool3(_182.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, _182.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, _182.z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _192 = false;
    }
    bool _202 = false;
    if (_192)
    {
        float4 _195 = smoothstep(_7_colorRed, _7_colorGreen, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _202 = all(bool4(_195.x == float4(1.0f, 0.0f, 1.0f, 1.0f).x, _195.y == float4(1.0f, 0.0f, 1.0f, 1.0f).y, _195.z == float4(1.0f, 0.0f, 1.0f, 1.0f).z, _195.w == float4(1.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _202 = false;
    }
    float4 _203 = 0.0f.xxxx;
    if (_202)
    {
        _203 = _7_colorGreen;
    }
    else
    {
        _203 = _7_colorRed;
    }
    return _203;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
