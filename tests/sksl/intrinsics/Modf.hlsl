cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 value = float4(2.5f, -2.5f, 8.0f, -0.125f);
    float4 expectedWhole = float4(2.0f, -2.0f, 8.0f, 0.0f);
    float4 expectedFraction = float4(0.5f, -0.5f, 0.0f, -0.125f);
    bool4 ok = bool4(false, false, false, false);
    float4 whole = 0.0f.xxxx;
    float _55 = 0.0f;
    float _48 = modf(value.x, _55);
    whole.x = _55;
    float4 fraction = 0.0f.xxxx;
    fraction.x = _48;
    bool _66 = false;
    if (whole.x == 2.0f)
    {
        _66 = fraction.x == 0.5f;
    }
    else
    {
        _66 = false;
    }
    ok.x = _66;
    float2 _72 = 0.0f.xx;
    float2 _69 = modf(value.xy, _72);
    whole = float4(_72.x, _72.y, whole.z, whole.w);
    fraction = float4(_69.x, _69.y, fraction.z, fraction.w);
    bool _91 = false;
    if (all(bool2(whole.xy.x == float2(2.0f, -2.0f).x, whole.xy.y == float2(2.0f, -2.0f).y)))
    {
        _91 = all(bool2(fraction.xy.x == float2(0.5f, -0.5f).x, fraction.xy.y == float2(0.5f, -0.5f).y));
    }
    else
    {
        _91 = false;
    }
    ok.y = _91;
    float3 _98 = 0.0f.xxx;
    float3 _94 = modf(value.xyz, _98);
    whole = float4(_98.x, _98.y, _98.z, whole.w);
    fraction = float4(_94.x, _94.y, _94.z, fraction.w);
    bool _118 = false;
    if (all(bool3(whole.xyz.x == float3(2.0f, -2.0f, 8.0f).x, whole.xyz.y == float3(2.0f, -2.0f, 8.0f).y, whole.xyz.z == float3(2.0f, -2.0f, 8.0f).z)))
    {
        _118 = all(bool3(fraction.xyz.x == float3(0.5f, -0.5f, 0.0f).x, fraction.xyz.y == float3(0.5f, -0.5f, 0.0f).y, fraction.xyz.z == float3(0.5f, -0.5f, 0.0f).z));
    }
    else
    {
        _118 = false;
    }
    ok.z = _118;
    float4 _121 = modf(value, whole);
    fraction = _121;
    bool _133 = false;
    if (all(bool4(whole.x == expectedWhole.x, whole.y == expectedWhole.y, whole.z == expectedWhole.z, whole.w == expectedWhole.w)))
    {
        _133 = all(bool4(fraction.x == expectedFraction.x, fraction.y == expectedFraction.y, fraction.z == expectedFraction.z, fraction.w == expectedFraction.w));
    }
    else
    {
        _133 = false;
    }
    ok.w = _133;
    float4 _138 = 0.0f.xxxx;
    if (all(ok))
    {
        _138 = _10_colorGreen;
    }
    else
    {
        _138 = _10_colorRed;
    }
    return _138;
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
