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
    int counter = 0;
    for (int i = 0; i < 10; i++)
    {
        if (i == 0)
        {
            continue;
        }
        counter += 10;
    }
    float4 _49 = 0.0f.xxxx;
    if (counter == 90)
    {
        _49 = _7_colorGreen;
    }
    else
    {
        _49 = _7_colorRed;
    }
    return _49;
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
