cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_a : packoffset(c0);
    float _10_b : packoffset(c0.y);
    float _10_c : packoffset(c0.z);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor.x = mad(_10_a, _10_b, _10_c);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
