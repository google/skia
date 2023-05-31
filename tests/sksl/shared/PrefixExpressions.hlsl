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
    int i = 5;
    int _34 = 5 + 1;
    i = _34;
    bool _40 = false;
    if (true)
    {
        _40 = _34 == 6;
    }
    else
    {
        _40 = false;
    }
    ok = _40;
    bool _46 = false;
    if (_40)
    {
        int _43 = _34 + 1;
        i = _43;
        _46 = _43 == 7;
    }
    else
    {
        _46 = false;
    }
    ok = _46;
    bool _52 = false;
    if (_46)
    {
        int _49 = i;
        int _50 = _49 - 1;
        i = _50;
        _52 = _50 == 6;
    }
    else
    {
        _52 = false;
    }
    ok = _52;
    int _53 = i;
    int _54 = _53 - 1;
    i = _54;
    bool _58 = false;
    if (_52)
    {
        _58 = _54 == 5;
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    float f = 0.5f;
    float _63 = 0.5f + 1.0f;
    f = _63;
    bool _68 = false;
    if (_58)
    {
        _68 = _63 == 1.5f;
    }
    else
    {
        _68 = false;
    }
    ok = _68;
    bool _74 = false;
    if (_68)
    {
        float _71 = _63 + 1.0f;
        f = _71;
        _74 = _71 == 2.5f;
    }
    else
    {
        _74 = false;
    }
    ok = _74;
    bool _80 = false;
    if (_74)
    {
        float _77 = f;
        float _78 = _77 - 1.0f;
        f = _78;
        _80 = _78 == 1.5f;
    }
    else
    {
        _80 = false;
    }
    ok = _80;
    float _81 = f;
    float _82 = _81 - 1.0f;
    f = _82;
    bool _86 = false;
    if (_80)
    {
        _86 = _82 == 0.5f;
    }
    else
    {
        _86 = false;
    }
    ok = _86;
    bool _96 = false;
    if (_86)
    {
        _96 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _96 = false;
    }
    ok = _96;
    uint _103 = uint(_10_colorGreen.x);
    uint val = _103;
    uint2 _108 = uint2(_103, ~_103);
    uint2 mask = _108;
    uint2 _112 = ~_108;
    int2 _117 = int2(int(_112.x), int(_112.y));
    int2 imask = _117;
    int2 _119 = ~_117;
    uint2 _125 = (~_108) & uint2(uint(_119.x), uint(_119.y));
    mask = _125;
    bool _133 = false;
    if (_96)
    {
        _133 = all(bool2(_125.x == uint2(0u, 0u).x, _125.y == uint2(0u, 0u).y));
    }
    else
    {
        _133 = false;
    }
    ok = _133;
    float one = _10_colorGreen.x;
    float4 _141 = float4(_10_colorGreen.x, 0.0f, 0.0f, 0.0f);
    float4 _142 = float4(0.0f, _10_colorGreen.x, 0.0f, 0.0f);
    float4 _143 = float4(0.0f, 0.0f, _10_colorGreen.x, 0.0f);
    float4 _144 = float4(0.0f, 0.0f, 0.0f, _10_colorGreen.x);
    float4x4 m = float4x4(_141, _142, _143, _144);
    float4 _146 = 0.0f.xxxx;
    if (_133)
    {
        _146 = mul(-_10_colorGreen, float4x4(-_141, -_142, -_143, -_144));
    }
    else
    {
        _146 = _10_colorRed;
    }
    return _146;
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
