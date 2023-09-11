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
    bool _38 = false;
    if (true)
    {
        _38 = _7_colorGreen.y == 1.0f;
    }
    else
    {
        _38 = false;
    }
    ok = _38;
    bool _45 = false;
    if (_38)
    {
        _45 = _7_colorGreen.x != 1.0f;
    }
    else
    {
        _45 = false;
    }
    ok = _45;
    bool _58 = false;
    if (_45)
    {
        _58 = all(bool2(_7_colorGreen.yx.x == _7_colorRed.xy.x, _7_colorGreen.yx.y == _7_colorRed.xy.y));
    }
    else
    {
        _58 = false;
    }
    ok = _58;
    bool _69 = false;
    if (_58)
    {
        _69 = all(bool2(_7_colorGreen.yx.x == _7_colorRed.xy.x, _7_colorGreen.yx.y == _7_colorRed.xy.y));
    }
    else
    {
        _69 = false;
    }
    ok = _69;
    bool _90 = false;
    if (_69)
    {
        bool _89 = false;
        if (all(bool2(_7_colorGreen.yx.x == _7_colorRed.xy.x, _7_colorGreen.yx.y == _7_colorRed.xy.y)))
        {
            _89 = true;
        }
        else
        {
            _89 = _7_colorGreen.w != _7_colorRed.w;
        }
        _90 = _89;
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    bool _111 = false;
    if (_90)
    {
        bool _110 = false;
        if (any(bool2(_7_colorGreen.yx.x != _7_colorRed.xy.x, _7_colorGreen.yx.y != _7_colorRed.xy.y)))
        {
            _110 = _7_colorGreen.w == _7_colorRed.w;
        }
        else
        {
            _110 = false;
        }
        _111 = _110;
    }
    else
    {
        _111 = false;
    }
    ok = _111;
    float4 _112 = 0.0f.xxxx;
    if (_111)
    {
        _112 = _7_colorGreen;
    }
    else
    {
        _112 = _7_colorRed;
    }
    return _112;
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
