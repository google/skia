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
    bool ok = true;
    bool _41 = false;
    if (true)
    {
        _41 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    uint _48 = uint(_10_colorGreen.x);
    uint val = _48;
    uint2 _53 = uint2(_48, ~_48);
    uint2 mask = _53;
    uint2 _57 = ~_53;
    int2 _62 = int2(int(_57.x), int(_57.y));
    int2 imask = _62;
    int2 _64 = ~_62;
    uint2 _70 = (~_53) & uint2(uint(_64.x), uint(_64.y));
    mask = _70;
    bool _78 = false;
    if (_41)
    {
        _78 = all(bool2(_70.x == uint2(0u, 0u).x, _70.y == uint2(0u, 0u).y));
    }
    else
    {
        _78 = false;
    }
    ok = _78;
    float one = _10_colorGreen.x;
    float4 _87 = float4(_10_colorGreen.x, 0.0f, 0.0f, 0.0f);
    float4 _88 = float4(0.0f, _10_colorGreen.x, 0.0f, 0.0f);
    float4 _89 = float4(0.0f, 0.0f, _10_colorGreen.x, 0.0f);
    float4 _90 = float4(0.0f, 0.0f, 0.0f, _10_colorGreen.x);
    float4x4 m = float4x4(_87, _88, _89, _90);
    float4 _92 = 0.0f.xxxx;
    if (_78)
    {
        _92 = mul(-_10_colorGreen, float4x4(-_87, -_88, -_89, -_90));
    }
    else
    {
        _92 = _10_colorRed;
    }
    return _92;
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
