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
    int s = int(_10_unknownInput);
    int i = int(_10_unknownInput);
    uint us = uint(_10_unknownInput);
    uint ui = uint(_10_unknownInput);
    float h = _10_unknownInput;
    float f = _10_unknownInput;
    int s2s = s;
    int i2s = i;
    int us2s = int(us);
    int ui2s = int(ui);
    int h2s = int(h);
    int f2s = int(f);
    int b2s = int(b);
    int s2i = s;
    int i2i = i;
    int us2i = int(us);
    int ui2i = int(ui);
    int h2i = int(h);
    int f2i = int(f);
    int b2i = int(b);
    uint s2us = uint(s);
    uint i2us = uint(i);
    uint us2us = us;
    uint ui2us = ui;
    uint h2us = uint(h);
    uint f2us = uint(f);
    uint b2us = uint(b);
    uint s2ui = uint(s);
    uint i2ui = uint(i);
    uint us2ui = us;
    uint ui2ui = ui;
    uint h2ui = uint(h);
    uint f2ui = uint(f);
    uint b2ui = uint(b);
    float s2f = float(s);
    float i2f = float(i);
    float us2f = float(us);
    float ui2f = float(ui);
    float h2f = h;
    float f2f = f;
    float b2f = float(b);
    sk_FragColor.x = (((((((((((((((((((((float(s) + float(i)) + float(us)) + float(ui)) + h) + f) + float(s2s)) + float(i2s)) + float(us2s)) + float(ui2s)) + float(h2s)) + float(f2s)) + float(b2s)) + float(s2i)) + float(i2i)) + float(us2i)) + float(ui2i)) + float(h2i)) + float(f2i)) + float(b2i)) + float(s2us)) + float(i2us)) + float(us2us);
    sk_FragColor.x += (((((((((((((((((float(ui2us) + float(h2us)) + float(f2us)) + float(b2us)) + float(s2ui)) + float(i2ui)) + float(us2ui)) + float(ui2ui)) + float(h2ui)) + float(f2ui)) + float(b2ui)) + s2f) + i2f) + us2f) + ui2f) + h2f) + f2f) + b2f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
