cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    int i1 = 0;
    i1 = 0 + 1;
    int i2 = 4660;
    i2 = 4660 + 1;
    int i3 = 32766;
    i3 = 32766 + 1;
    int i4 = -32766;
    i4 = (-32766) + 1;
    int i5 = 19132;
    i5 = 19132 + 1;
    return _7_colorGreen;
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
