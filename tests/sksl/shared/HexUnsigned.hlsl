cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    uint u1 = 0u;
    u1++;
    uint u2 = 305441741u;
    u2++;
    uint u3 = 2147483646u;
    u3++;
    uint u4 = 4294967294u;
    u4++;
    uint u5 = 65534u;
    u5++;
    return _10_colorGreen;
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
