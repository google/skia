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
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    a = 1;
    b = 2;
    c = 5;
    bool _41 = false;
    if (true)
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
    bool _47 = false;
    if (_44)
    {
        _47 = true;
    }
    else
    {
        _47 = false;
    }
    float4 _48 = 0.0f.xxxx;
    if (_47)
    {
        _48 = _11_colorGreen;
    }
    else
    {
        _48 = _11_colorRed;
    }
    return _48;
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
