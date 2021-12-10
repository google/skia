cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float _10_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
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
    bool _69 = false;
    if ((_40 > 4.0f) == (_40 < 2.0f))
    {
        _69 = true;
    }
    else
    {
        bool _68 = false;
        if (2.0f >= _10_unknownInput)
        {
            _68 = _42 <= _40;
        }
        else
        {
            _68 = false;
        }
        _69 = _68;
    }
    bool b = _69;
    bool _73 = _10_unknownInput > 2.0f;
    bool c = _73;
    bool _75 = _69 != _73;
    bool d = _75;
    bool _79 = false;
    if (_69)
    {
        _79 = _73;
    }
    else
    {
        _79 = false;
    }
    bool e = _79;
    bool _83 = false;
    if (_69)
    {
        _83 = true;
    }
    else
    {
        _83 = _73;
    }
    bool f = _83;
    float _85 = _40 + 12.0f;
    x = _85;
    float _86 = _85 - 12.0f;
    x = _86;
    float _88 = _42 / 10.0f;
    y = _88;
    x = _86 * _88;
    int _91 = _50 | 0;
    z = _91;
    int _93 = _91 & (-1);
    z = _93;
    int _94 = _93 ^ 0;
    z = _94;
    int _95 = _94 >> 2;
    z = _95;
    int _96 = _95 << 4;
    z = _96;
    z = _96 % 5;
    float _104 = float(6);
    x = _104;
    y = 6.0f;
    z = 6;
    int2 _122 = (~5).xx;
    int2 w = _122;
    int2 _123 = ~_122;
    w = _123;
    bool _130 = false;
    if (_123.x == 5)
    {
        _130 = _123.y == 5;
    }
    else
    {
        _130 = false;
    }
    bool _134 = false;
    if (_130)
    {
        _134 = _104 == 6.0f;
    }
    else
    {
        _134 = false;
    }
    bool _137 = false;
    if (_134)
    {
        _137 = true;
    }
    else
    {
        _137 = false;
    }
    bool _140 = false;
    if (_137)
    {
        _140 = true;
    }
    else
    {
        _140 = false;
    }
    float4 _141 = 0.0f.xxxx;
    if (_140)
    {
        _141 = _10_colorGreen;
    }
    else
    {
        _141 = _10_colorRed;
    }
    return _141;
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
