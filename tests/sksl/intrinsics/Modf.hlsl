cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 value = float4(2.5f, -2.5f, 8.0f, -0.125f);
    bool4 ok = bool4(false, false, false, false);
    float4 whole = 0.0f.xxxx;
    float _43 = 0.0f;
    float _38 = modf(2.5f, _43);
    whole.x = _43;
    float4 fraction = 0.0f.xxxx;
    fraction.x = _38;
    bool _56 = false;
    if (whole.x == 2.0f)
    {
        _56 = fraction.x == 0.5f;
    }
    else
    {
        _56 = false;
    }
    ok.x = _56;
    float2 _62 = 0.0f.xx;
    float2 _59 = modf(value.xy, _62);
    float4 _64 = whole;
    whole = float4(_62.x, _62.y, _64.z, _64.w);
    float4 _66 = fraction;
    fraction = float4(_59.x, _59.y, _66.z, _66.w);
    bool _81 = false;
    if (all(bool2(float4(_62.x, _62.y, _64.z, _64.w).xy.x == float2(2.0f, -2.0f).x, float4(_62.x, _62.y, _64.z, _64.w).xy.y == float2(2.0f, -2.0f).y)))
    {
        _81 = all(bool2(float4(_59.x, _59.y, _66.z, _66.w).xy.x == float2(0.5f, -0.5f).x, float4(_59.x, _59.y, _66.z, _66.w).xy.y == float2(0.5f, -0.5f).y));
    }
    else
    {
        _81 = false;
    }
    ok.y = _81;
    float3 _88 = 0.0f.xxx;
    float3 _84 = modf(value.xyz, _88);
    float4 _91 = whole;
    whole = float4(_88.x, _88.y, _88.z, _91.w);
    float4 _93 = fraction;
    fraction = float4(_84.x, _84.y, _84.z, _93.w);
    bool _106 = false;
    if (all(bool3(float4(_88.x, _88.y, _88.z, _91.w).xyz.x == float3(2.0f, -2.0f, 8.0f).x, float4(_88.x, _88.y, _88.z, _91.w).xyz.y == float3(2.0f, -2.0f, 8.0f).y, float4(_88.x, _88.y, _88.z, _91.w).xyz.z == float3(2.0f, -2.0f, 8.0f).z)))
    {
        _106 = all(bool3(float4(_84.x, _84.y, _84.z, _93.w).xyz.x == float3(0.5f, -0.5f, 0.0f).x, float4(_84.x, _84.y, _84.z, _93.w).xyz.y == float3(0.5f, -0.5f, 0.0f).y, float4(_84.x, _84.y, _84.z, _93.w).xyz.z == float3(0.5f, -0.5f, 0.0f).z));
    }
    else
    {
        _106 = false;
    }
    ok.z = _106;
    float4 _111 = 0.0f.xxxx;
    float4 _109 = modf(value, _111);
    whole = _111;
    fraction = _109;
    bool _121 = false;
    if (all(bool4(_111.x == float4(2.0f, -2.0f, 8.0f, 0.0f).x, _111.y == float4(2.0f, -2.0f, 8.0f, 0.0f).y, _111.z == float4(2.0f, -2.0f, 8.0f, 0.0f).z, _111.w == float4(2.0f, -2.0f, 8.0f, 0.0f).w)))
    {
        _121 = all(bool4(_109.x == float4(0.5f, -0.5f, 0.0f, -0.125f).x, _109.y == float4(0.5f, -0.5f, 0.0f, -0.125f).y, _109.z == float4(0.5f, -0.5f, 0.0f, -0.125f).z, _109.w == float4(0.5f, -0.5f, 0.0f, -0.125f).w));
    }
    else
    {
        _121 = false;
    }
    ok.w = _121;
    float4 _126 = 0.0f.xxxx;
    if (all(ok))
    {
        _126 = _7_colorGreen;
    }
    else
    {
        _126 = _7_colorRed;
    }
    return _126;
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
