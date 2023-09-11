cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_a : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int b = 0;

void frag_main()
{
    sk_FragColor.x = ldexp(_10_a, b);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
