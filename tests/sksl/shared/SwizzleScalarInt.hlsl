cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    int i = int(_10_unknownInput);
    int4 i4 = i.xxxx;
    i4 = int4(i.xx, 0, 1);
    i4 = int4(0, i, 1, 0);
    i4 = int4(0, i, 0, i);
    return float4(float(i4.x), float(i4.y), float(i4.z), float(i4.w));
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
