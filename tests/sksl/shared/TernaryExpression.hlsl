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
    bool _41 = false;
    if (true)
    {
        _41 = (_10_colorGreen.y == 1.0f) ? true : false;
    }
    else
    {
        _41 = false;
    }
    ok = _41;
    bool _49 = false;
    if (_41)
    {
        _49 = (_10_colorGreen.x == 1.0f) ? false : true;
    }
    else
    {
        _49 = false;
    }
    ok = _49;
    bool _63 = false;
    if (_49)
    {
        _63 = all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y)) ? true : false;
    }
    else
    {
        _63 = false;
    }
    ok = _63;
    bool _75 = false;
    if (_63)
    {
        _75 = any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y)) ? false : true;
    }
    else
    {
        _75 = false;
    }
    ok = _75;
    float4 _76 = 0.0f.xxxx;
    if (_75)
    {
        _76 = _10_colorGreen;
    }
    else
    {
        _76 = _10_colorRed;
    }
    return _76;
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
