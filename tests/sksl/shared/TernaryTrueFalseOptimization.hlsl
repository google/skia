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
    bool _47 = false;
    if (_40)
    {
        _47 = _10_colorGreen.x != 1.0f;
    }
    else
    {
        _47 = false;
    }
    ok = _47;
    bool _60 = false;
    if (_47)
    {
        _60 = all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y));
    }
    else
    {
        _60 = false;
    }
    ok = _60;
    bool _71 = false;
    if (_60)
    {
        _71 = all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y));
    }
    else
    {
        _71 = false;
    }
    ok = _71;
    bool _92 = false;
    if (_71)
    {
        bool _91 = false;
        if (all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y)))
        {
            _91 = true;
        }
        else
        {
            _91 = _10_colorGreen.w != _10_colorRed.w;
        }
        _92 = _91;
    }
    else
    {
        _92 = false;
    }
    ok = _92;
    bool _113 = false;
    if (_92)
    {
        bool _112 = false;
        if (any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y)))
        {
            _112 = _10_colorGreen.w == _10_colorRed.w;
        }
        else
        {
            _112 = false;
        }
        _113 = _112;
    }
    else
    {
        _113 = false;
    }
    ok = _113;
    float4 _114 = 0.0f.xxxx;
    if (_113)
    {
        _114 = _10_colorGreen;
    }
    else
    {
        _114 = _10_colorRed;
    }
    return _114;
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
