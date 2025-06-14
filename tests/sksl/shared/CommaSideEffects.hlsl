cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorRed : packoffset(c0);
    float4 _12_colorGreen : packoffset(c1);
    float4 _12_colorWhite : packoffset(c2);
    float4 _12_colorBlack : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void setToColorBlack_vh4(out float4 _27)
{
    _27 = _12_colorBlack;
}

float4 main(float2 _34)
{
    float4 b = _12_colorRed;
    float4 c = _12_colorGreen;
    float4 _46 = 0.0f.xxxx;
    setToColorBlack_vh4(_46);
    float4 d = _46;
    float4 a = _12_colorWhite;
    float4 _52 = _12_colorWhite * _12_colorWhite;
    a = _52;
    float4 _53 = _12_colorRed * _12_colorRed;
    b = _53;
    float4 _54 = _12_colorGreen * _12_colorGreen;
    c = _54;
    float4 _55 = _46 * _46;
    d = _55;
    bool _69 = false;
    if (all(bool4(_52.x == _12_colorWhite.x, _52.y == _12_colorWhite.y, _52.z == _12_colorWhite.z, _52.w == _12_colorWhite.w)))
    {
        _69 = all(bool4(_53.x == _12_colorRed.x, _53.y == _12_colorRed.y, _53.z == _12_colorRed.z, _53.w == _12_colorRed.w));
    }
    else
    {
        _69 = false;
    }
    bool _76 = false;
    if (_69)
    {
        _76 = all(bool4(_54.x == _12_colorGreen.x, _54.y == _12_colorGreen.y, _54.z == _12_colorGreen.z, _54.w == _12_colorGreen.w));
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        _83 = all(bool4(_55.x == _12_colorBlack.x, _55.y == _12_colorBlack.y, _55.z == _12_colorBlack.z, _55.w == _12_colorBlack.w));
    }
    else
    {
        _83 = false;
    }
    float4 _84 = 0.0f.xxxx;
    if (_83)
    {
        _84 = _12_colorGreen;
    }
    else
    {
        _84 = _12_colorRed;
    }
    return _84;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
