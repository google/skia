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
    bool _37 = _11_colorGreen.y != 0.0f;
    bool TRUE = _37;
    bool _44 = false;
    if (true)
    {
        _44 = 1 == int(_37);
    }
    else
    {
        _44 = false;
    }
    ok = _44;
    bool _50 = false;
    if (_44)
    {
        _50 = 1.0f == float(_37);
    }
    else
    {
        _50 = false;
    }
    ok = _50;
    bool _53 = false;
    if (_50)
    {
        _53 = _37;
    }
    else
    {
        _53 = false;
    }
    ok = _53;
    bool _58 = false;
    if (_53)
    {
        _58 = 1 == int(_37);
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    bool _63 = false;
    if (_58)
    {
        _63 = 1.0f == float(_37);
    }
    else
    {
        _63 = false;
    }
    ok = _63;
    bool _71 = false;
    if (_63)
    {
        bool2 _68 = _37.xx;
        _71 = all(bool2(bool2(true, true).x == _68.x, bool2(true, true).y == _68.y));
    }
    else
    {
        _71 = false;
    }
    ok = _71;
    bool _80 = false;
    if (_71)
    {
        int2 _77 = int(_37).xx;
        _80 = all(bool2(int2(1, 1).x == _77.x, int2(1, 1).y == _77.y));
    }
    else
    {
        _80 = false;
    }
    ok = _80;
    bool _88 = false;
    if (_80)
    {
        float2 _85 = float(_37).xx;
        _88 = all(bool2(1.0f.xx.x == _85.x, 1.0f.xx.y == _85.y));
    }
    else
    {
        _88 = false;
    }
    ok = _88;
    float4 _89 = 0.0f.xxxx;
    if (_88)
    {
        _89 = _11_colorGreen;
    }
    else
    {
        _89 = _11_colorRed;
    }
    return _89;
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
