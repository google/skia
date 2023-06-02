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
    float f = _10_colorGreen.y;
    int _39 = int(_10_colorGreen.y);
    int i = _39;
    bool _45 = _10_colorGreen.y != 0.0f;
    bool b = _45;
    float f1 = _10_colorGreen.y;
    float _48 = float(_39);
    float f2 = _48;
    float _50 = float(_45);
    float f3 = _50;
    int _53 = int(_10_colorGreen.y);
    int i1 = _53;
    int i2 = _39;
    int _56 = int(_45);
    int i3 = _56;
    bool _59 = _10_colorGreen.y != 0.0f;
    bool b1 = _59;
    bool _61 = _39 != 0;
    bool b2 = _61;
    bool b3 = _45;
    float4 _79 = 0.0f.xxxx;
    if (((((((((_10_colorGreen.y + _48) + _50) + float(_53)) + float(_39)) + float(_56)) + float(_59)) + float(_61)) + float(_45)) == 9.0f)
    {
        _79 = _10_colorGreen;
    }
    else
    {
        _79 = _10_colorRed;
    }
    return _79;
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
