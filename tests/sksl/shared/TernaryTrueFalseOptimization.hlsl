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
    bool _40 = false;
    if (true)
    {
        _40 = _10_colorGreen.y == 1.0f;
    }
    else
    {
        _40 = false;
    }
    ok = _40;
    bool _48 = false;
    if (_40)
    {
        _48 = !(_10_colorGreen.x == 1.0f);
    }
    else
    {
        _48 = false;
    }
    ok = _48;
    bool _61 = false;
    if (_48)
    {
        _61 = all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y));
    }
    else
    {
        _61 = false;
    }
    ok = _61;
    bool _73 = false;
    if (_61)
    {
        _73 = !any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y));
    }
    else
    {
        _73 = false;
    }
    ok = _73;
    bool _94 = false;
    if (_73)
    {
        bool _93 = false;
        if (all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y)))
        {
            _93 = true;
        }
        else
        {
            _93 = _10_colorGreen.w != _10_colorRed.w;
        }
        _94 = _93;
    }
    else
    {
        _94 = false;
    }
    ok = _94;
    bool _115 = false;
    if (_94)
    {
        bool _114 = false;
        if (any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y)))
        {
            _114 = _10_colorGreen.w == _10_colorRed.w;
        }
        else
        {
            _114 = false;
        }
        _115 = _114;
    }
    else
    {
        _115 = false;
    }
    ok = _115;
    float4 _116 = 0.0f.xxxx;
    if (_115)
    {
        _116 = _10_colorGreen;
    }
    else
    {
        _116 = _10_colorRed;
    }
    return _116;
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
