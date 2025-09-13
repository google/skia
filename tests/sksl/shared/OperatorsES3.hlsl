cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float _11_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float x = 1.0f;
    float y = 2.0f;
    int z = 3;
    float _40 = (1.0f - 1.0f) + (((2.0f * 1.0f) * 1.0f) * (2.0f - 1.0f));
    x = _40;
    float _42 = (_40 / 2.0f) / _40;
    y = _42;
    int _50 = ((((3 / 2) % 3) << 4) >> 2) << 1;
    z = _50;
    bool _70 = false;
    if ((_40 > 4.0f) == (_40 < 2.0f))
    {
        _70 = true;
    }
    else
    {
        bool _69 = false;
        if (2.0f >= _11_unknownInput)
        {
            _69 = _42 <= _40;
        }
        else
        {
            _69 = false;
        }
        _70 = _69;
    }
    bool b = _70;
    bool _74 = _11_unknownInput > 2.0f;
    bool c = _74;
    bool _76 = _70 != _74;
    bool d = _76;
    bool _80 = false;
    if (_70)
    {
        _80 = _74;
    }
    else
    {
        _80 = false;
    }
    bool e = _80;
    bool _84 = false;
    if (_70)
    {
        _84 = true;
    }
    else
    {
        _84 = _74;
    }
    bool f = _84;
    float _86 = _40 + 12.0f;
    x = _86;
    float _87 = _86 - 12.0f;
    x = _87;
    float _89 = _42 * 0.100000001490116119384765625f;
    y = _89;
    x = _87 * _89;
    int _92 = _50 | 0;
    z = _92;
    int _94 = _92 & (-1);
    z = _94;
    int _95 = _94 ^ 0;
    z = _95;
    int _96 = _95 >> 2;
    z = _96;
    int _97 = _96 << 4;
    z = _97;
    z = _97 % 5;
    float _105 = float(6);
    x = _105;
    y = 6.0f;
    z = 6;
    int2 w = int2(-6, -6);
    int2 _124 = ~int2(-6, -6);
    w = _124;
    bool _131 = false;
    if (_124.x == 5)
    {
        _131 = _124.y == 5;
    }
    else
    {
        _131 = false;
    }
    bool _135 = false;
    if (_131)
    {
        _135 = _105 == 6.0f;
    }
    else
    {
        _135 = false;
    }
    bool _138 = false;
    if (_135)
    {
        _138 = true;
    }
    else
    {
        _138 = false;
    }
    bool _141 = false;
    if (_138)
    {
        _141 = true;
    }
    else
    {
        _141 = false;
    }
    float4 _142 = 0.0f.xxxx;
    if (_141)
    {
        _142 = _11_colorGreen;
    }
    else
    {
        _142 = _11_colorRed;
    }
    return _142;
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
