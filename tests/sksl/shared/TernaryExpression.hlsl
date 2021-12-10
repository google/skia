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
    bool ok = true;
    bool _42 = false;
    if (ok)
    {
        _42 = (_10_colorGreen.y == 1.0f) ? true : false;
    }
    else
    {
        _42 = false;
    }
    ok = _42;
    bool _51 = false;
    if (ok)
    {
        _51 = (_10_colorGreen.x == 1.0f) ? false : true;
    }
    else
    {
        _51 = false;
    }
    ok = _51;
    float4 _53 = 0.0f.xxxx;
    if (ok)
    {
        _53 = _10_colorGreen;
    }
    else
    {
        _53 = _10_colorRed;
    }
    return _53;
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
