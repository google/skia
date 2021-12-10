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
    float4 d = 0.0f.xxxx;
    setToColorBlack_vh4(d);
    float4 a = _11_colorWhite;
    a *= a;
    b *= b;
    c *= c;
    d *= d;
    bool _76 = false;
    if (all(bool4(a.x == _11_colorWhite.x, a.y == _11_colorWhite.y, a.z == _11_colorWhite.z, a.w == _11_colorWhite.w)))
    {
        _76 = all(bool4(b.x == _11_colorRed.x, b.y == _11_colorRed.y, b.z == _11_colorRed.z, b.w == _11_colorRed.w));
    }
    else
    {
        _76 = false;
    }
    bool _84 = false;
    if (_76)
    {
        _84 = all(bool4(c.x == _11_colorGreen.x, c.y == _11_colorGreen.y, c.z == _11_colorGreen.z, c.w == _11_colorGreen.w));
    }
    else
    {
        _84 = false;
    }
    bool _92 = false;
    if (_84)
    {
        _92 = all(bool4(d.x == _11_colorBlack.x, d.y == _11_colorBlack.y, d.z == _11_colorBlack.z, d.w == _11_colorBlack.w));
    }
    else
    {
        _92 = false;
    }
    float4 _93 = 0.0f.xxxx;
    if (_92)
    {
        _93 = _11_colorGreen;
    }
    else
    {
        _93 = _11_colorRed;
    }
    return _93;
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
