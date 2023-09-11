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
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    a = 1;
    b = 2;
    c = 5;
    bool _38 = false;
    if (true)
    {
        _38 = true;
    }
    else
    {
        _38 = false;
    }
    bool _41 = false;
    if (_38)
    {
        _41 = true;
    }
    else
    {
        _41 = false;
    }
    bool _44 = false;
    if (_41)
    {
        _44 = true;
    }
    else
    {
        _44 = false;
    }
    float4 _45 = 0.0f.xxxx;
    if (_44)
    {
        _45 = _7_colorGreen;
    }
    else
    {
        _45 = _7_colorRed;
    }
    return _45;
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
