cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 color = 0.0f.xxxx;
    if (_7_colorWhite.x == 1.0f)
    {
        color.y = 1.0f;
    }
    if (_7_colorWhite.x == 2.0f)
    {
    }
    else
    {
        color.w = 1.0f;
    }
    while (_7_colorWhite.x == 2.0f)
    {
    }
    do
    {
    } while (_7_colorWhite.x == 2.0f);
    return color;
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
