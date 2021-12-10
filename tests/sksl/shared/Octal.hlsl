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
    bool _43 = false;
    if (i1 == 1)
    {
        _43 = i2 == 342391;
    }
    else
    {
        _43 = false;
    }
    bool _48 = false;
    if (_43)
    {
        _48 = i3 == 2000000000;
    }
    else
    {
        _48 = false;
    }
    bool _53 = false;
    if (_48)
    {
        _53 = i4 == (-2000000000);
    }
    else
    {
        _53 = false;
    }
    float4 _54 = 0.0f.xxxx;
    if (_53)
    {
        _54 = _10_colorGreen;
    }
    else
    {
        _54 = _10_colorRed;
    }
    return _54;
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
