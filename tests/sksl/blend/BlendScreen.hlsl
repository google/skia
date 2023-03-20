cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_src : packoffset(c0);
    float4 _10_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = _10_src + ((1.0f.xxxx - _10_src) * _10_dst);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
