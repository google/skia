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
    uint _46 = uint(_10_colorGreen.y);
    uint u = _46;
    bool _52 = _10_colorGreen.y != 0.0f;
    bool b = _52;
    float f1 = _10_colorGreen.y;
    float _55 = float(_39);
    float f2 = _55;
    float _57 = float(_46);
    float f3 = _57;
    float _59 = float(_52);
    float f4 = _59;
    int _62 = int(_10_colorGreen.y);
    int i1 = _62;
    int i2 = _39;
    int _65 = int(_46);
    int i3 = _65;
    int _67 = int(_52);
    int i4 = _67;
    uint _70 = uint(_10_colorGreen.y);
    uint u1 = _70;
    uint _72 = uint(_39);
    uint u2 = _72;
    uint u3 = _46;
    uint _75 = uint(_52);
    uint u4 = _75;
    bool _79 = _10_colorGreen.y != 0.0f;
    bool b1 = _79;
    bool _81 = _39 != 0;
    bool b2 = _81;
    bool _83 = _46 != 0u;
    bool b3 = _83;
    bool b4 = _52;
    float4 _114 = 0.0f.xxxx;
    if ((((((((((((((((_10_colorGreen.y + _55) + _57) + _59) + float(_62)) + float(_39)) + float(_65)) + float(_67)) + float(_70)) + float(_72)) + float(_46)) + float(_75)) + float(_79)) + float(_81)) + float(_83)) + float(_52)) == 16.0f)
    {
        _114 = _10_colorGreen;
    }
    else
    {
        _114 = _10_colorRed;
    }
    return _114;
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
