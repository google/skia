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
    float f = _7_colorGreen.y;
    int _36 = int(_7_colorGreen.y);
    int i = _36;
    uint _43 = uint(_7_colorGreen.y);
    uint u = _43;
    bool _50 = _7_colorGreen.y != 0.0f;
    bool b = _50;
    float f1 = _7_colorGreen.y;
    float _53 = float(_36);
    float f2 = _53;
    float _55 = float(_43);
    float f3 = _55;
    float _57 = float(_50);
    float f4 = _57;
    int _60 = int(_7_colorGreen.y);
    int i1 = _60;
    int i2 = _36;
    int _63 = int(_43);
    int i3 = _63;
    int _65 = int(_50);
    int i4 = _65;
    uint _68 = uint(_7_colorGreen.y);
    uint u1 = _68;
    uint _70 = uint(_36);
    uint u2 = _70;
    uint u3 = _43;
    uint _73 = uint(_50);
    uint u4 = _73;
    bool _77 = _7_colorGreen.y != 0.0f;
    bool b1 = _77;
    bool _79 = _36 != 0;
    bool b2 = _79;
    bool _81 = _43 != 0u;
    bool b3 = _81;
    bool b4 = _50;
    float4 _112 = 0.0f.xxxx;
    if ((((((((((((((((_7_colorGreen.y + _53) + _55) + _57) + float(_60)) + float(_36)) + float(_63)) + float(_65)) + float(_68)) + float(_70)) + float(_43)) + float(_73)) + float(_77)) + float(_79)) + float(_81)) + float(_50)) == 16.0f)
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
