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
    int _29[2] = { 1, 2 };
    int i2[2] = _29;
    int _31[2] = { 1, 2 };
    int s2[2] = _31;
    float _37[2] = { 1.0f, 2.0f };
    float f2[2] = _37;
    float _39[2] = { 1.0f, 2.0f };
    float h2[2] = _39;
    i2 = _31;
    s2 = _31;
    f2 = _39;
    h2 = _39;
    float cf2[2] = _37;
    bool _48 = false;
    if (true && true)
    {
        _48 = true && true;
    }
    else
    {
        _48 = false;
    }
    bool _52 = false;
    if (_48)
    {
        _52 = true && true;
    }
    else
    {
        _52 = false;
    }
    bool _56 = false;
    if (_52)
    {
        _56 = true && true;
    }
    else
    {
        _56 = false;
    }
    float4 _57 = 0.0f.xxxx;
    if (_56)
    {
        _57 = _7_colorGreen;
    }
    else
    {
        _57 = _7_colorRed;
    }
    return _57;
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
