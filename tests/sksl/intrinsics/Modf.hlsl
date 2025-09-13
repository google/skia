cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 value = float4(2.5f, -2.5f, 8.0f, -0.125f);
    bool4 ok = bool4(false, false, false, false);
    float4 whole = 0.0f.xxxx;
    float _46 = 0.0f;
    float _42 = modf(2.5f, _46);
    whole.x = _46;
    float4 fraction = 0.0f.xxxx;
    fraction.x = _42;
    bool _59 = false;
    if (whole.x == 2.0f)
    {
        _59 = fraction.x == 0.5f;
    }
    else
    {
        _59 = false;
    }
    ok.x = _59;
    float2 _65 = 0.0f.xx;
    float2 _62 = modf(value.xy, _65);
    float4 _67 = whole;
    whole = float4(_65.x, _65.y, _67.z, _67.w);
    float4 _69 = fraction;
    fraction = float4(_62.x, _62.y, _69.z, _69.w);
    bool _84 = false;
    if (all(bool2(float4(_65.x, _65.y, _67.z, _67.w).xy.x == float2(2.0f, -2.0f).x, float4(_65.x, _65.y, _67.z, _67.w).xy.y == float2(2.0f, -2.0f).y)))
    {
        _84 = all(bool2(float4(_62.x, _62.y, _69.z, _69.w).xy.x == float2(0.5f, -0.5f).x, float4(_62.x, _62.y, _69.z, _69.w).xy.y == float2(0.5f, -0.5f).y));
    }
    else
    {
        _84 = false;
    }
    ok.y = _84;
    float3 _91 = 0.0f.xxx;
    float3 _87 = modf(value.xyz, _91);
    float4 _94 = whole;
    whole = float4(_91.x, _91.y, _91.z, _94.w);
    float4 _96 = fraction;
    fraction = float4(_87.x, _87.y, _87.z, _96.w);
    bool _109 = false;
    if (all(bool3(float4(_91.x, _91.y, _91.z, _94.w).xyz.x == float3(2.0f, -2.0f, 8.0f).x, float4(_91.x, _91.y, _91.z, _94.w).xyz.y == float3(2.0f, -2.0f, 8.0f).y, float4(_91.x, _91.y, _91.z, _94.w).xyz.z == float3(2.0f, -2.0f, 8.0f).z)))
    {
        _109 = all(bool3(float4(_87.x, _87.y, _87.z, _96.w).xyz.x == float3(0.5f, -0.5f, 0.0f).x, float4(_87.x, _87.y, _87.z, _96.w).xyz.y == float3(0.5f, -0.5f, 0.0f).y, float4(_87.x, _87.y, _87.z, _96.w).xyz.z == float3(0.5f, -0.5f, 0.0f).z));
    }
    else
    {
        _109 = false;
    }
    ok.z = _109;
    float4 _114 = 0.0f.xxxx;
    float4 _112 = modf(value, _114);
    whole = _114;
    fraction = _112;
    bool _124 = false;
    if (all(bool4(_114.x == float4(2.0f, -2.0f, 8.0f, 0.0f).x, _114.y == float4(2.0f, -2.0f, 8.0f, 0.0f).y, _114.z == float4(2.0f, -2.0f, 8.0f, 0.0f).z, _114.w == float4(2.0f, -2.0f, 8.0f, 0.0f).w)))
    {
        _124 = all(bool4(_112.x == float4(0.5f, -0.5f, 0.0f, -0.125f).x, _112.y == float4(0.5f, -0.5f, 0.0f, -0.125f).y, _112.z == float4(0.5f, -0.5f, 0.0f, -0.125f).z, _112.w == float4(0.5f, -0.5f, 0.0f, -0.125f).w));
    }
    else
    {
        _124 = false;
    }
    ok.w = _124;
    float4 _129 = 0.0f.xxxx;
    if (all(ok))
    {
        _129 = _11_colorGreen;
    }
    else
    {
        _129 = _11_colorRed;
    }
    return _129;
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
