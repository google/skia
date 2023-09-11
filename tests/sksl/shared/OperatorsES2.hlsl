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
    int _45 = (((3 / 2) * 3) + 4) - 2;
    z = _45;
    bool _65 = false;
    if ((_37 > 4.0f) == (_37 < 2.0f))
    {
        _65 = true;
    }
    else
    {
        bool _64 = false;
        if (2.0f >= _7_unknownInput)
        {
            _64 = _39 <= _37;
        }
        else
        {
            _64 = false;
        }
        _65 = _64;
    }
    bool b = _65;
    bool _69 = _7_unknownInput > 2.0f;
    bool c = _69;
    bool _71 = _65 != _69;
    bool d = _71;
    bool _75 = false;
    if (_65)
    {
        _75 = _69;
    }
    else
    {
        _75 = false;
    }
    bool e = _75;
    bool _79 = false;
    if (_65)
    {
        _79 = true;
    }
    else
    {
        _79 = _69;
    }
    bool f = _79;
    float _81 = _37 + 12.0f;
    x = _81;
    float _82 = _81 - 12.0f;
    x = _82;
    float _84 = _39 * 0.100000001490116119384765625f;
    y = _84;
    x = _82 * _84;
    x = 6.0f;
    y = (((float(_65) * float(_69)) * float(_71)) * float(_75)) * float(_79);
    y = 6.0f;
    z = _45 - 1;
    z = 6;
    bool _101 = false;
    if (true)
    {
        _101 = true;
    }
    else
    {
        _101 = false;
    }
    bool _104 = false;
    if (_101)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    float4 _105 = 0.0f.xxxx;
    if (_104)
    {
        _105 = _7_colorGreen;
    }
    else
    {
        _105 = _7_colorRed;
    }
    return _105;
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
