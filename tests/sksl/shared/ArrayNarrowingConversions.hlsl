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
    bool _51 = false;
    if (true && true)
    {
        _51 = true && true;
    }
    else
    {
        _51 = false;
    }
    bool _55 = false;
    if (_51)
    {
        _55 = true && true;
    }
    else
    {
        _55 = false;
    }
    bool _59 = false;
    if (_55)
    {
        _59 = true && true;
    }
    else
    {
        _59 = false;
    }
    float4 _60 = 0.0f.xxxx;
    if (_59)
    {
        _60 = _11_colorGreen;
    }
    else
    {
        _60 = _11_colorRed;
    }
    return _60;
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
