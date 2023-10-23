cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float _7_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float x = 1.0f;
    float y = 2.0f;
    int z = 3;
    float _37 = (1.0f - 1.0f) + (((2.0f * 1.0f) * 1.0f) * (2.0f - 1.0f));
    x = _37;
    float _39 = (_37 / 2.0f) / _37;
    y = _39;
    int _47 = ((((3 / 2) % 3) << 4) >> 2) << 1;
    z = _47;
    bool _67 = false;
    if ((_37 > 4.0f) == (_37 < 2.0f))
    {
        _67 = true;
    }
    else
    {
        bool _66 = false;
        if (2.0f >= _7_unknownInput)
        {
            _66 = _39 <= _37;
        }
        else
        {
            _66 = false;
        }
        _67 = _66;
    }
    bool b = _67;
    bool _71 = _7_unknownInput > 2.0f;
    bool c = _71;
    bool _73 = _67 != _71;
    bool d = _73;
    bool _77 = false;
    if (_67)
    {
        _77 = _71;
    }
    else
    {
        _77 = false;
    }
    bool e = _77;
    bool _81 = false;
    if (_67)
    {
        _81 = true;
    }
    else
    {
        _81 = _71;
    }
    bool f = _81;
    float _83 = _37 + 12.0f;
    x = _83;
    float _84 = _83 - 12.0f;
    x = _84;
    float _86 = _39 * 0.100000001490116119384765625f;
    y = _86;
    x = _84 * _86;
    int _89 = _47 | 0;
    z = _89;
    int _91 = _89 & (-1);
    z = _91;
    int _92 = _91 ^ 0;
    z = _92;
    int _93 = _92 >> 2;
    z = _93;
    int _94 = _93 << 4;
    z = _94;
    z = _94 % 5;
    float _102 = float(6);
    x = _102;
    y = 6.0f;
    z = 6;
    int2 w = int2(-6, -6);
    int2 _121 = ~int2(-6, -6);
    w = _121;
    bool _128 = false;
    if (_121.x == 5)
    {
        _128 = _121.y == 5;
    }
    else
    {
        _128 = false;
    }
    bool _132 = false;
    if (_128)
    {
        _132 = _102 == 6.0f;
    }
    else
    {
        _132 = false;
    }
    bool _135 = false;
    if (_132)
    {
        _135 = true;
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
    float4 _139 = 0.0f.xxxx;
    if (_138)
    {
        _139 = _7_colorGreen;
    }
    else
    {
        _139 = _7_colorRed;
    }
    return _139;
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
