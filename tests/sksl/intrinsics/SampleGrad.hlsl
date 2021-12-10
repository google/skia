cbuffer sksl_synthetic_uniforms : register(b0, space0)
{
    float2 _34_u_skRTFlip : packoffset(c1024);
};

Texture2D<float4> t : register(t0, space0);
SamplerState _t_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    return t.SampleGrad(_t_sampler, _25, ddx(_25), ddy(_25) * _34_u_skRTFlip.y.xx);
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
