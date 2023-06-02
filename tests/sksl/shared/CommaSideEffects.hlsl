cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorRed : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorWhite : packoffset(c2);
    float4 _11_colorBlack : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void setToColorBlack_vh4(out float4 _26)
{
    _26 = _11_colorBlack;
}

float4 main(float2 _34)
{
    float4 b = _11_colorRed;
    float4 c = _11_colorGreen;
    float4 _46 = 0.0f.xxxx;
    setToColorBlack_vh4(_46);
    float4 d = _46;
    float4 a = _11_colorWhite;
    float4 _52 = _11_colorWhite * _11_colorWhite;
    a = _52;
    float4 _53 = _11_colorRed * _11_colorRed;
    b = _53;
    float4 _54 = _11_colorGreen * _11_colorGreen;
    c = _54;
    float4 _55 = _46 * _46;
    d = _55;
    bool _68 = false;
    if (all(bool4(_52.x == _11_colorWhite.x, _52.y == _11_colorWhite.y, _52.z == _11_colorWhite.z, _52.w == _11_colorWhite.w)))
    {
        _68 = all(bool4(_53.x == _11_colorRed.x, _53.y == _11_colorRed.y, _53.z == _11_colorRed.z, _53.w == _11_colorRed.w));
    }
    else
    {
        _68 = false;
    }
    bool _75 = false;
    if (_68)
    {
        _75 = all(bool4(_54.x == _11_colorGreen.x, _54.y == _11_colorGreen.y, _54.z == _11_colorGreen.z, _54.w == _11_colorGreen.w));
    }
    else
    {
        _75 = false;
    }
    bool _82 = false;
    if (_75)
    {
        _82 = all(bool4(_55.x == _11_colorBlack.x, _55.y == _11_colorBlack.y, _55.z == _11_colorBlack.z, _55.w == _11_colorBlack.w));
    }
    else
    {
        _82 = false;
    }
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _11_colorGreen;
    }
    else
    {
        _83 = _11_colorRed;
    }
    return _83;
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
