cbuffer _UniformBuffer : register(b0, space0)
{
    float _11_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool b = true;
    int _27 = int(_11_unknownInput);
    int s = _27;
    int _31 = int(_11_unknownInput);
    int i = _31;
    uint _37 = uint(_11_unknownInput);
    uint us = _37;
    uint _41 = uint(_11_unknownInput);
    uint ui = _41;
    float h = _11_unknownInput;
    float f = _11_unknownInput;
    int s2s = _27;
    int i2s = _31;
    int _52 = int(_37);
    int us2s = _52;
    int _54 = int(_41);
    int ui2s = _54;
    int _56 = int(_11_unknownInput);
    int h2s = _56;
    int _58 = int(_11_unknownInput);
    int f2s = _58;
    int _60 = int(true);
    int b2s = _60;
    int s2i = _27;
    int i2i = _31;
    int _65 = int(_37);
    int us2i = _65;
    int _67 = int(_41);
    int ui2i = _67;
    int _69 = int(_11_unknownInput);
    int h2i = _69;
    int _71 = int(_11_unknownInput);
    int f2i = _71;
    int _73 = int(true);
    int b2i = _73;
    uint _75 = uint(_27);
    uint s2us = _75;
    uint _77 = uint(_31);
    uint i2us = _77;
    uint us2us = _37;
    uint ui2us = _41;
    uint h2us = uint(_11_unknownInput);
    uint f2us = uint(_11_unknownInput);
    uint b2us = uint(true);
    uint s2ui = uint(_27);
    uint i2ui = uint(_31);
    uint us2ui = _37;
    uint ui2ui = _41;
    uint h2ui = uint(_11_unknownInput);
    uint f2ui = uint(_11_unknownInput);
    uint b2ui = uint(true);
    float s2f = float(_27);
    float i2f = float(_31);
    float us2f = float(_37);
    float ui2f = float(_41);
    float h2f = _11_unknownInput;
    float f2f = _11_unknownInput;
    float b2f = float(true);
    sk_FragColor.x = (((((((((((((((((((((float(_27) + float(_31)) + float(_37)) + float(_41)) + _11_unknownInput) + _11_unknownInput) + float(_27)) + float(_31)) + float(_52)) + float(_54)) + float(_56)) + float(_58)) + float(_60)) + float(_27)) + float(_31)) + float(_65)) + float(_67)) + float(_69)) + float(_71)) + float(_73)) + float(_75)) + float(_77)) + float(_37);
    sk_FragColor.x += (((((((((((((((((float(ui2us) + float(h2us)) + float(f2us)) + float(b2us)) + float(s2ui)) + float(i2ui)) + float(us2ui)) + float(ui2ui)) + float(h2ui)) + float(f2ui)) + float(b2ui)) + s2f) + i2f) + us2f) + ui2f) + h2f) + f2f) + b2f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
