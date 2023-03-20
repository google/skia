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
    int _32[2] = { 1, 2 };
    int i2[2] = _32;
    int _34[2] = { 1, 2 };
    int s2[2] = _34;
    float _40[2] = { 1.0f, 2.0f };
    float f2[2] = _40;
    float _42[2] = { 1.0f, 2.0f };
    float h2[2] = _42;
    i2 = _34;
    s2 = _34;
    f2 = _42;
    h2 = _42;
    float cf2[2] = _40;
    bool _50 = false;
    if (true && true)
    {
        _50 = true && true;
    }
    else
    {
        _50 = false;
    }
    bool _54 = false;
    if (_50)
    {
        _54 = true && true;
    }
    else
    {
        _54 = false;
    }
    bool _58 = false;
    if (_54)
    {
        _58 = true && true;
    }
    else
    {
        _58 = false;
    }
    float4 _59 = 0.0f.xxxx;
    if (_58)
    {
        _59 = _10_colorGreen;
    }
    else
    {
        _59 = _10_colorRed;
    }
    return _59;
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
