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
    float4 _32 = _7_colorGreen.yyyy * 6.0f;
    float4 value = _32;
    int4 _exp = int4(0, 0, 0, 0);
    int _45 = 0;
    float _41 = frexp(_32.x, _45);
    _exp.x = _45;
    float4 result = 0.0f.xxxx;
    result.x = _41;
    bool _60 = false;
    if (result.x == 0.75f)
    {
        _60 = _exp.x == 3;
    }
    else
    {
        _60 = false;
    }
    bool4 ok = bool4(false, false, false, false);
    ok.x = _60;
    int2 _66 = int2(0, 0);
    float2 _63 = frexp(value.xy, _66);
    int4 _70 = _exp;
    _exp = int4(_66.x, _66.y, _70.z, _70.w);
    float4 _72 = result;
    result = float4(_63.x, _63.y, _72.z, _72.w);
    bool _80 = false;
    if (float4(_63.x, _63.y, _72.z, _72.w).y == 0.75f)
    {
        _80 = int4(_66.x, _66.y, _70.z, _70.w).y == 3;
    }
    else
    {
        _80 = false;
    }
    ok.y = _80;
    int3 _87 = int3(0, 0, 0);
    float3 _83 = frexp(value.xyz, _87);
    int4 _91 = _exp;
    _exp = int4(_87.x, _87.y, _87.z, _91.w);
    float4 _93 = result;
    result = float4(_83.x, _83.y, _83.z, _93.w);
    bool _101 = false;
    if (float4(_83.x, _83.y, _83.z, _93.w).z == 0.75f)
    {
        _101 = int4(_87.x, _87.y, _87.z, _91.w).z == 3;
    }
    else
    {
        _101 = false;
    }
    ok.z = _101;
    int4 _106 = int4(0, 0, 0, 0);
    float4 _104 = frexp(value, _106);
    _exp = _106;
    result = _104;
    bool _114 = false;
    if (_104.w == 0.75f)
    {
        _114 = _106.w == 3;
    }
    else
    {
        _114 = false;
    }
    ok.w = _114;
    float4 _118 = 0.0f.xxxx;
    if (all(ok))
    {
        _118 = _7_colorGreen;
    }
    else
    {
        _118 = _7_colorRed;
    }
    return _118;
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
