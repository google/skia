cbuffer _UniformBuffer : register(b0, space0)
{
    float2 _10_ah : packoffset(c0);
    float2 _10_bh : packoffset(c0.z);
    float2 _10_af : packoffset(c1);
    float2 _10_bf : packoffset(c1.z);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor.x = (_10_ah.x * _10_bh.y) - (_10_ah.y * _10_bh.x);
    sk_FragColor.y = (_10_af.x * _10_bf.y) - (_10_af.y * _10_bf.x);
    sk_FragColor.z = 12.0f;
    sk_FragColor = float4(float3(-8.0f, -8.0f, 12.0f).x, float3(-8.0f, -8.0f, 12.0f).y, float3(-8.0f, -8.0f, 12.0f).z, sk_FragColor.w);
    sk_FragColor = float4(sk_FragColor.x, float3(9.0f, -18.0f, -9.0f).x, float3(9.0f, -18.0f, -9.0f).y, float3(9.0f, -18.0f, -9.0f).z);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
