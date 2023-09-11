cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_src : packoffset(c0);
    float4 _7_dst : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = float4(((_7_dst.xyz * (1.0f - _7_src.w)) + (_7_src.xyz * (1.0f - _7_dst.w))) + (_7_src.xyz * _7_dst.xyz), _7_src.w + ((1.0f - _7_src.w) * _7_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
