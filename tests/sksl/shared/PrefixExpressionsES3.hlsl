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
    bool ok = true;
    uint _39 = uint(_11_colorGreen.x);
    uint val = _39;
    uint2 _44 = uint2(_39, ~_39);
    uint2 mask = _44;
    uint2 _48 = ~_44;
    int2 _53 = int2(int(_48.x), int(_48.y));
    int2 imask = _53;
    int2 _55 = ~_53;
    uint2 _61 = (~_44) & uint2(uint(_55.x), uint(_55.y));
    mask = _61;
    bool _70 = false;
    if (true)
    {
        _70 = all(bool2(_61.x == uint2(0u, 0u).x, _61.y == uint2(0u, 0u).y));
    }
    else
    {
        _70 = false;
    }
    ok = _70;
    float4 _71 = 0.0f.xxxx;
    if (_70)
    {
        _71 = _11_colorGreen;
    }
    else
    {
        _71 = _11_colorRed;
    }
    return _71;
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
