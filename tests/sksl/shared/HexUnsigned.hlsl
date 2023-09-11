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
    uint u1 = 0u;
    u1 = 0u + 1u;
    uint u2 = 305441741u;
    u2 = 305441741u + 1u;
    uint u3 = 2147483646u;
    u3 = 2147483646u + 1u;
    uint u4 = 4294967294u;
    u4 = 4294967294u + 1u;
    uint u5 = 65534u;
    u5 = 65534u + 1u;
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
