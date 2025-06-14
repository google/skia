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
    int _48 = (((3 / 2) * 3) + 4) - 2;
    z = _48;
    bool _68 = false;
    if ((_40 > 4.0f) == (_40 < 2.0f))
    {
        _68 = true;
    }
    else
    {
        bool _67 = false;
        if (2.0f >= _11_unknownInput)
        {
            _67 = _42 <= _40;
        }
        else
        {
            _67 = false;
        }
        _68 = _67;
    }
    bool b = _68;
    bool _72 = _11_unknownInput > 2.0f;
    bool c = _72;
    bool _74 = _68 != _72;
    bool d = _74;
    bool _78 = false;
    if (_68)
    {
        _78 = _72;
    }
    else
    {
        _78 = false;
    }
    bool e = _78;
    bool _82 = false;
    if (_68)
    {
        _82 = true;
    }
    else
    {
        _82 = _72;
    }
    bool f = _82;
    float _84 = _40 + 12.0f;
    x = _84;
    float _85 = _84 - 12.0f;
    x = _85;
    float _87 = _42 * 0.100000001490116119384765625f;
    y = _87;
    x = _85 * _87;
    x = 6.0f;
    y = (((float(_68) * float(_72)) * float(_74)) * float(_78)) * float(_82);
    y = 6.0f;
    z = _48 - 1;
    z = 6;
    bool _104 = false;
    if (true)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    bool _107 = false;
    if (_104)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    float4 _108 = 0.0f.xxxx;
    if (_107)
    {
        _108 = _11_colorGreen;
    }
    else
    {
        _108 = _11_colorRed;
    }
    return _108;
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
