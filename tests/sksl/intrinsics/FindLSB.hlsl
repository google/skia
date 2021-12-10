cbuffer _UniformBuffer : register(b0, space0)
{
    int _10_a : packoffset(c0);
    uint _10_b : packoffset(c0.y);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor.x = float(firstbitlow(_10_a));
    sk_FragColor.y = float(int(firstbitlow(_10_b)));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
