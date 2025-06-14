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
    float f = _11_colorGreen.y;
    int _39 = int(_11_colorGreen.y);
    int i = _39;
    bool _46 = _11_colorGreen.y != 0.0f;
    bool b = _46;
    float f1 = _11_colorGreen.y;
    float _49 = float(_39);
    float f2 = _49;
    float _51 = float(_46);
    float f3 = _51;
    int _54 = int(_11_colorGreen.y);
    int i1 = _54;
    int i2 = _39;
    int _57 = int(_46);
    int i3 = _57;
    bool _60 = _11_colorGreen.y != 0.0f;
    bool b1 = _60;
    bool _62 = _39 != 0;
    bool b2 = _62;
    bool b3 = _46;
    float4 _80 = 0.0f.xxxx;
    if (((((((((_11_colorGreen.y + _49) + _51) + float(_54)) + float(_39)) + float(_57)) + float(_60)) + float(_62)) + float(_46)) == 9.0f)
    {
        _80 = _11_colorGreen;
    }
    else
    {
        _80 = _11_colorRed;
    }
    return _80;
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
