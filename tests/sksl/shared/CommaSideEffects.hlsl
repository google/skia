cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorRed : packoffset(c0);
    float4 _8_colorGreen : packoffset(c1);
    float4 _8_colorWhite : packoffset(c2);
    float4 _8_colorBlack : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void setToColorBlack_vh4(out float4 _23)
{
    _23 = _8_colorBlack;
}

float4 main(float2 _31)
{
    float4 b = _8_colorRed;
    float4 c = _8_colorGreen;
    float4 _43 = 0.0f.xxxx;
    setToColorBlack_vh4(_43);
    float4 d = _43;
    float4 a = _8_colorWhite;
    float4 _49 = _8_colorWhite * _8_colorWhite;
    a = _49;
    float4 _50 = _8_colorRed * _8_colorRed;
    b = _50;
    float4 _51 = _8_colorGreen * _8_colorGreen;
    c = _51;
    float4 _52 = _43 * _43;
    d = _52;
    bool _66 = false;
    if (all(bool4(_49.x == _8_colorWhite.x, _49.y == _8_colorWhite.y, _49.z == _8_colorWhite.z, _49.w == _8_colorWhite.w)))
    {
        _66 = all(bool4(_50.x == _8_colorRed.x, _50.y == _8_colorRed.y, _50.z == _8_colorRed.z, _50.w == _8_colorRed.w));
    }
    else
    {
        _66 = false;
    }
    bool _73 = false;
    if (_66)
    {
        _73 = all(bool4(_51.x == _8_colorGreen.x, _51.y == _8_colorGreen.y, _51.z == _8_colorGreen.z, _51.w == _8_colorGreen.w));
    }
    else
    {
        _73 = false;
    }
    bool _80 = false;
    if (_73)
    {
        _80 = all(bool4(_52.x == _8_colorBlack.x, _52.y == _8_colorBlack.y, _52.z == _8_colorBlack.z, _52.w == _8_colorBlack.w));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _8_colorGreen;
    }
    else
    {
        _81 = _8_colorRed;
    }
    return _81;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
