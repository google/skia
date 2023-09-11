cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool b = true;
    int _24 = int(_7_unknownInput);
    int s = _24;
    int _28 = int(_7_unknownInput);
    int i = _28;
    uint _34 = uint(_7_unknownInput);
    uint us = _34;
    uint _38 = uint(_7_unknownInput);
    uint ui = _38;
    float h = _7_unknownInput;
    float f = _7_unknownInput;
    int s2s = _24;
    int i2s = _28;
    int _49 = int(_34);
    int us2s = _49;
    int _51 = int(_38);
    int ui2s = _51;
    int _53 = int(_7_unknownInput);
    int h2s = _53;
    int _55 = int(_7_unknownInput);
    int f2s = _55;
    int _57 = int(true);
    int b2s = _57;
    int s2i = _24;
    int i2i = _28;
    int _62 = int(_34);
    int us2i = _62;
    int _64 = int(_38);
    int ui2i = _64;
    int _66 = int(_7_unknownInput);
    int h2i = _66;
    int _68 = int(_7_unknownInput);
    int f2i = _68;
    int _70 = int(true);
    int b2i = _70;
    uint _72 = uint(_24);
    uint s2us = _72;
    uint _74 = uint(_28);
    uint i2us = _74;
    uint us2us = _34;
    uint ui2us = _38;
    uint h2us = uint(_7_unknownInput);
    uint f2us = uint(_7_unknownInput);
    uint b2us = uint(true);
    uint s2ui = uint(_24);
    uint i2ui = uint(_28);
    uint us2ui = _34;
    uint ui2ui = _38;
    uint h2ui = uint(_7_unknownInput);
    uint f2ui = uint(_7_unknownInput);
    uint b2ui = uint(true);
    float s2f = float(_24);
    float i2f = float(_28);
    float us2f = float(_34);
    float ui2f = float(_38);
    float h2f = _7_unknownInput;
    float f2f = _7_unknownInput;
    float b2f = float(true);
    sk_FragColor.x = (((((((((((((((((((((float(_24) + float(_28)) + float(_34)) + float(_38)) + _7_unknownInput) + _7_unknownInput) + float(_24)) + float(_28)) + float(_49)) + float(_51)) + float(_53)) + float(_55)) + float(_57)) + float(_24)) + float(_28)) + float(_62)) + float(_64)) + float(_66)) + float(_68)) + float(_70)) + float(_72)) + float(_74)) + float(_34);
    sk_FragColor.x += (((((((((((((((((float(ui2us) + float(h2us)) + float(f2us)) + float(b2us)) + float(s2ui)) + float(i2ui)) + float(us2ui)) + float(ui2ui)) + float(h2ui)) + float(f2ui)) + float(b2ui)) + s2f) + i2f) + us2f) + ui2f) + h2f) + f2f) + b2f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
