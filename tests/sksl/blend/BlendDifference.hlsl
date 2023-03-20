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
    sk_FragColor = float4((_10_src.xyz + _10_dst.xyz) - (min(_10_src.xyz * _10_dst.w, _10_dst.xyz * _10_src.w) * 2.0f), _10_src.w + ((1.0f - _10_src.w) * _10_dst.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
