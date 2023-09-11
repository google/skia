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
    bool _43 = _7_colorGreen.y != 0.0f;
    bool b = _43;
    float f1 = _7_colorGreen.y;
    float _46 = float(_36);
    float f2 = _46;
    float _48 = float(_43);
    float f3 = _48;
    int _51 = int(_7_colorGreen.y);
    int i1 = _51;
    int i2 = _36;
    int _54 = int(_43);
    int i3 = _54;
    bool _57 = _7_colorGreen.y != 0.0f;
    bool b1 = _57;
    bool _59 = _36 != 0;
    bool b2 = _59;
    bool b3 = _43;
    float4 _77 = 0.0f.xxxx;
    if (((((((((_7_colorGreen.y + _46) + _48) + float(_51)) + float(_36)) + float(_54)) + float(_57)) + float(_59)) + float(_43)) == 9.0f)
    {
        _77 = _7_colorGreen;
    }
    else
    {
        _77 = _7_colorRed;
    }
    return _77;
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
