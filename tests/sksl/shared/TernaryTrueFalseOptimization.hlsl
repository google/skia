cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool ok = true;
    bool _41 = false;
    if (true)
    {
        _41 = _11_colorGreen.y == 1.0f;
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    bool _48 = false;
    if (_41)
    {
        _48 = _11_colorGreen.x != 1.0f;
    }
    else
    {
        _48 = false;
    }
    ok = _48;
    bool _61 = false;
    if (_48)
    {
        _61 = all(bool2(_11_colorGreen.yx.x == _11_colorRed.xy.x, _11_colorGreen.yx.y == _11_colorRed.xy.y));
    }
    else
    {
        _61 = false;
    }
    ok = _61;
    bool _72 = false;
    if (_61)
    {
        _72 = all(bool2(_11_colorGreen.yx.x == _11_colorRed.xy.x, _11_colorGreen.yx.y == _11_colorRed.xy.y));
    }
    else
    {
        _72 = false;
    }
    ok = _72;
    bool _93 = false;
    if (_72)
    {
        bool _92 = false;
        if (all(bool2(_11_colorGreen.yx.x == _11_colorRed.xy.x, _11_colorGreen.yx.y == _11_colorRed.xy.y)))
        {
            _92 = true;
        }
        else
        {
            _92 = _11_colorGreen.w != _11_colorRed.w;
        }
        _93 = _92;
    }
    else
    {
        _93 = false;
    }
    ok = _93;
    bool _114 = false;
    if (_93)
    {
        bool _113 = false;
        if (any(bool2(_11_colorGreen.yx.x != _11_colorRed.xy.x, _11_colorGreen.yx.y != _11_colorRed.xy.y)))
        {
            _113 = _11_colorGreen.w == _11_colorRed.w;
        }
        else
        {
            _113 = false;
        }
        _114 = _113;
    }
    else
    {
        _114 = false;
    }
    ok = _114;
    float4 _115 = 0.0f.xxxx;
    if (_114)
    {
        _115 = _11_colorGreen;
    }
    else
    {
        _115 = _11_colorRed;
    }
    return _115;
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
