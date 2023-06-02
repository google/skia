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
    float4 _35 = _10_colorGreen.yyyy * 6.0f;
    float4 value = _35;
    int4 _exp = int4(0, 0, 0, 0);
    int _47 = 0;
    float _43 = frexp(_35.x, _47);
    _exp.x = _47;
    float4 result = 0.0f.xxxx;
    result.x = _43;
    bool _62 = false;
    if (result.x == 0.75f)
    {
        _62 = _exp.x == 3;
    }
    else
    {
        _62 = false;
    }
    bool4 ok = bool4(false, false, false, false);
    ok.x = _62;
    int2 _68 = int2(0, 0);
    float2 _65 = frexp(value.xy, _68);
    int4 _72 = _exp;
    _exp = int4(_68.x, _68.y, _72.z, _72.w);
    float4 _74 = result;
    result = float4(_65.x, _65.y, _74.z, _74.w);
    bool _82 = false;
    if (float4(_65.x, _65.y, _74.z, _74.w).y == 0.75f)
    {
        _82 = int4(_68.x, _68.y, _72.z, _72.w).y == 3;
    }
    else
    {
        _82 = false;
    }
    ok.y = _82;
    int3 _89 = int3(0, 0, 0);
    float3 _85 = frexp(value.xyz, _89);
    int4 _93 = _exp;
    _exp = int4(_89.x, _89.y, _89.z, _93.w);
    float4 _95 = result;
    result = float4(_85.x, _85.y, _85.z, _95.w);
    bool _103 = false;
    if (float4(_85.x, _85.y, _85.z, _95.w).z == 0.75f)
    {
        _103 = int4(_89.x, _89.y, _89.z, _93.w).z == 3;
    }
    else
    {
        _103 = false;
    }
    ok.z = _103;
    int4 _108 = int4(0, 0, 0, 0);
    float4 _106 = frexp(value, _108);
    _exp = _108;
    result = _106;
    bool _116 = false;
    if (_106.w == 0.75f)
    {
        _116 = _108.w == 3;
    }
    else
    {
        _116 = false;
    }
    ok.w = _116;
    float4 _120 = 0.0f.xxxx;
    if (all(ok))
    {
        _120 = _10_colorGreen;
    }
    else
    {
        _120 = _10_colorRed;
    }
    return _120;
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
