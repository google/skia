cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool b = true;
    int _26 = int(_10_unknownInput);
    int s = _26;
    int _30 = int(_10_unknownInput);
    int i = _30;
    uint _36 = uint(_10_unknownInput);
    uint us = _36;
    uint _40 = uint(_10_unknownInput);
    uint ui = _40;
    float h = _10_unknownInput;
    float f = _10_unknownInput;
    int s2s = _26;
    int i2s = _30;
    int _51 = int(_36);
    int us2s = _51;
    int _53 = int(_40);
    int ui2s = _53;
    int _55 = int(_10_unknownInput);
    int h2s = _55;
    int _57 = int(_10_unknownInput);
    int f2s = _57;
    int _59 = int(true);
    int b2s = _59;
    int s2i = _26;
    int i2i = _30;
    int _64 = int(_36);
    int us2i = _64;
    int _66 = int(_40);
    int ui2i = _66;
    int _68 = int(_10_unknownInput);
    int h2i = _68;
    int _70 = int(_10_unknownInput);
    int f2i = _70;
    int _72 = int(true);
    int b2i = _72;
    uint _74 = uint(_26);
    uint s2us = _74;
    uint _76 = uint(_30);
    uint i2us = _76;
    uint us2us = _36;
    uint ui2us = _40;
    uint h2us = uint(_10_unknownInput);
    uint f2us = uint(_10_unknownInput);
    uint b2us = uint(true);
    uint s2ui = uint(_26);
    uint i2ui = uint(_30);
    uint us2ui = _36;
    uint ui2ui = _40;
    uint h2ui = uint(_10_unknownInput);
    uint f2ui = uint(_10_unknownInput);
    uint b2ui = uint(true);
    float s2f = float(_26);
    float i2f = float(_30);
    float us2f = float(_36);
    float ui2f = float(_40);
    float h2f = _10_unknownInput;
    float f2f = _10_unknownInput;
    float b2f = float(true);
    sk_FragColor.x = (((((((((((((((((((((float(_26) + float(_30)) + float(_36)) + float(_40)) + _10_unknownInput) + _10_unknownInput) + float(_26)) + float(_30)) + float(_51)) + float(_53)) + float(_55)) + float(_57)) + float(_59)) + float(_26)) + float(_30)) + float(_64)) + float(_66)) + float(_68)) + float(_70)) + float(_72)) + float(_74)) + float(_76)) + float(_36);
    sk_FragColor.x += (((((((((((((((((float(ui2us) + float(h2us)) + float(f2us)) + float(b2us)) + float(s2ui)) + float(i2ui)) + float(us2ui)) + float(ui2ui)) + float(h2ui)) + float(f2ui)) + float(b2ui)) + s2f) + i2f) + us2f) + ui2f) + h2f) + f2f) + b2f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
