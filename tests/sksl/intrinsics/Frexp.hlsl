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
    float4 value = _10_colorGreen.yyyy * 6.0f;
    int4 _exp = int4(0, 0, 0, 0);
    int _48 = 0;
    float _43 = frexp(value.x, _48);
    _exp.x = _48;
    float4 result = 0.0f.xxxx;
    result.x = _43;
    bool _63 = false;
    if (result.x == 0.75f)
    {
        _63 = _exp.x == 3;
    }
    else
    {
        _63 = false;
    }
    bool4 ok = bool4(false, false, false, false);
    ok.x = _63;
    int2 _69 = int2(0, 0);
    float2 _66 = frexp(value.xy, _69);
    _exp = int4(_69.x, _69.y, _exp.z, _exp.w);
    result = float4(_66.x, _66.y, result.z, result.w);
    bool _85 = false;
    if (result.y == 0.75f)
    {
        _85 = _exp.y == 3;
    }
    else
    {
        _85 = false;
    }
    ok.y = _85;
    int3 _92 = int3(0, 0, 0);
    float3 _88 = frexp(value.xyz, _92);
    _exp = int4(_92.x, _92.y, _92.z, _exp.w);
    result = float4(_88.x, _88.y, _88.z, result.w);
    bool _108 = false;
    if (result.z == 0.75f)
    {
        _108 = _exp.z == 3;
    }
    else
    {
        _108 = false;
    }
    ok.z = _108;
    float4 _111 = frexp(value, _exp);
    result = _111;
    bool _121 = false;
    if (result.w == 0.75f)
    {
        _121 = _exp.w == 3;
    }
    else
    {
        _121 = false;
    }
    ok.w = _121;
    float4 _125 = 0.0f.xxxx;
    if (all(ok))
    {
        _125 = _10_colorGreen;
    }
    else
    {
        _125 = _10_colorRed;
    }
    return _125;
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
