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
    int _48 = (((3 / 2) * 3) + 4) - 2;
    z = _48;
    bool _67 = false;
    if ((_40 > 4.0f) == (_40 < 2.0f))
    {
        _67 = true;
    }
    else
    {
        bool _66 = false;
        if (2.0f >= _10_unknownInput)
        {
            _66 = _42 <= _40;
        }
        else
        {
            _66 = false;
        }
        _67 = _66;
    }
    bool b = _67;
    bool _71 = _10_unknownInput > 2.0f;
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
    float _83 = _40 + 12.0f;
    x = _83;
    float _84 = _83 - 12.0f;
    x = _84;
    float _86 = _42 / 10.0f;
    y = _86;
    x = _84 * _86;
    x = 6.0f;
    y = (((float(_67) * float(_71)) * float(_73)) * float(_77)) * float(_81);
    y = 6.0f;
    z = _48 - 1;
    z = 6;
    bool _103 = false;
    if (true)
    {
        _103 = true;
    }
    else
    {
        _103 = false;
    }
    bool _106 = false;
    if (_103)
    {
        _106 = true;
    }
    else
    {
        _106 = false;
    }
    float4 _107 = 0.0f.xxxx;
    if (_106)
    {
        _107 = _10_colorGreen;
    }
    else
    {
        _107 = _10_colorRed;
    }
    return _107;
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
