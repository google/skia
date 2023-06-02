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
    int i1 = 1;
    int i2 = 342391;
    int i3 = 2000000000;
    int i4 = -2000000000;
    bool _40 = false;
    if (true)
    {
        _40 = true;
    }
    else
    {
        _40 = false;
    }
    bool _43 = false;
    if (_40)
    {
        _43 = true;
    }
    else
    {
        _43 = false;
    }
    bool _46 = false;
    if (_43)
    {
        _46 = true;
    }
    else
    {
        _46 = false;
    }
    float4 _47 = 0.0f.xxxx;
    if (_46)
    {
        _47 = _10_colorGreen;
    }
    else
    {
        _47 = _10_colorRed;
    }
    return _47;
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
