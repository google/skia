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
    bool4 ok = bool4(false, false, false, false);
    float4 whole = 0.0f.xxxx;
    float _45 = 0.0f;
    float _40 = modf(2.5f, _45);
    whole.x = _45;
    float4 fraction = 0.0f.xxxx;
    fraction.x = _40;
    bool _58 = false;
    if (whole.x == 2.0f)
    {
        _58 = fraction.x == 0.5f;
    }
    else
    {
        _58 = false;
    }
    ok.x = _58;
    float2 _64 = 0.0f.xx;
    float2 _61 = modf(value.xy, _64);
    float4 _66 = whole;
    whole = float4(_64.x, _64.y, _66.z, _66.w);
    float4 _68 = fraction;
    fraction = float4(_61.x, _61.y, _68.z, _68.w);
    bool _83 = false;
    if (all(bool2(float4(_64.x, _64.y, _66.z, _66.w).xy.x == float2(2.0f, -2.0f).x, float4(_64.x, _64.y, _66.z, _66.w).xy.y == float2(2.0f, -2.0f).y)))
    {
        _83 = all(bool2(float4(_61.x, _61.y, _68.z, _68.w).xy.x == float2(0.5f, -0.5f).x, float4(_61.x, _61.y, _68.z, _68.w).xy.y == float2(0.5f, -0.5f).y));
    }
    else
    {
        _83 = false;
    }
    ok.y = _83;
    float3 _90 = 0.0f.xxx;
    float3 _86 = modf(value.xyz, _90);
    float4 _93 = whole;
    whole = float4(_90.x, _90.y, _90.z, _93.w);
    float4 _95 = fraction;
    fraction = float4(_86.x, _86.y, _86.z, _95.w);
    bool _108 = false;
    if (all(bool3(float4(_90.x, _90.y, _90.z, _93.w).xyz.x == float3(2.0f, -2.0f, 8.0f).x, float4(_90.x, _90.y, _90.z, _93.w).xyz.y == float3(2.0f, -2.0f, 8.0f).y, float4(_90.x, _90.y, _90.z, _93.w).xyz.z == float3(2.0f, -2.0f, 8.0f).z)))
    {
        _108 = all(bool3(float4(_86.x, _86.y, _86.z, _95.w).xyz.x == float3(0.5f, -0.5f, 0.0f).x, float4(_86.x, _86.y, _86.z, _95.w).xyz.y == float3(0.5f, -0.5f, 0.0f).y, float4(_86.x, _86.y, _86.z, _95.w).xyz.z == float3(0.5f, -0.5f, 0.0f).z));
    }
    else
    {
        _108 = false;
    }
    ok.z = _108;
    float4 _113 = 0.0f.xxxx;
    float4 _111 = modf(value, _113);
    whole = _113;
    fraction = _111;
    bool _123 = false;
    if (all(bool4(_113.x == float4(2.0f, -2.0f, 8.0f, 0.0f).x, _113.y == float4(2.0f, -2.0f, 8.0f, 0.0f).y, _113.z == float4(2.0f, -2.0f, 8.0f, 0.0f).z, _113.w == float4(2.0f, -2.0f, 8.0f, 0.0f).w)))
    {
        _123 = all(bool4(_111.x == float4(0.5f, -0.5f, 0.0f, -0.125f).x, _111.y == float4(0.5f, -0.5f, 0.0f, -0.125f).y, _111.z == float4(0.5f, -0.5f, 0.0f, -0.125f).z, _111.w == float4(0.5f, -0.5f, 0.0f, -0.125f).w));
    }
    else
    {
        _123 = false;
    }
    ok.w = _123;
    float4 _128 = 0.0f.xxxx;
    if (all(ok))
    {
        _128 = _10_colorGreen;
    }
    else
    {
        _128 = _10_colorRed;
    }
    return _128;
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
