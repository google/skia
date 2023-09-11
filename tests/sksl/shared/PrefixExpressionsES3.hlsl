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
    bool ok = true;
    uint _36 = uint(_7_colorGreen.x);
    uint val = _36;
    uint2 _41 = uint2(_36, ~_36);
    uint2 mask = _41;
    uint2 _45 = ~_41;
    int2 _50 = int2(int(_45.x), int(_45.y));
    int2 imask = _50;
    int2 _52 = ~_50;
    uint2 _58 = (~_41) & uint2(uint(_52.x), uint(_52.y));
    mask = _58;
    bool _67 = false;
    if (true)
    {
        _67 = all(bool2(_58.x == uint2(0u, 0u).x, _58.y == uint2(0u, 0u).y));
    }
    else
    {
        _67 = false;
    }
    ok = _67;
    float4 _68 = 0.0f.xxxx;
    if (_67)
    {
        _68 = _7_colorGreen;
    }
    else
    {
        _68 = _7_colorRed;
    }
    return _68;
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
