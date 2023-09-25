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
    bool _34 = _7_colorGreen.y != 0.0f;
    bool TRUE = _34;
    bool _41 = false;
    if (true)
    {
        _41 = 1 == int(_34);
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    bool _47 = false;
    if (_41)
    {
        _47 = 1.0f == float(_34);
    }
    else
    {
        _47 = false;
    }
    ok = _47;
    bool _50 = false;
    if (_47)
    {
        _50 = _34;
    }
    else
    {
        _50 = false;
    }
    ok = _50;
    bool _55 = false;
    if (_50)
    {
        _55 = 1 == int(_34);
    }
    else
    {
        _55 = false;
    }
    ok = _55;
    bool _60 = false;
    if (_55)
    {
        _60 = 1.0f == float(_34);
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    bool _68 = false;
    if (_60)
    {
        bool2 _65 = _34.xx;
        _68 = all(bool2(bool2(true, true).x == _65.x, bool2(true, true).y == _65.y));
    }
    else
    {
        _68 = false;
    }
    ok = _68;
    bool _77 = false;
    if (_68)
    {
        int2 _74 = int(_34).xx;
        _77 = all(bool2(int2(1, 1).x == _74.x, int2(1, 1).y == _74.y));
    }
    else
    {
        _77 = false;
    }
    ok = _77;
    bool _85 = false;
    if (_77)
    {
        float2 _82 = float(_34).xx;
        _85 = all(bool2(1.0f.xx.x == _82.x, 1.0f.xx.y == _82.y));
    }
    else
    {
        _85 = false;
    }
    ok = _85;
    float4 _86 = 0.0f.xxxx;
    if (_85)
    {
        _86 = _7_colorGreen;
    }
    else
    {
        _86 = _7_colorRed;
    }
    return _86;
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
