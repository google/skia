cbuffer sksl_synthetic_uniforms : register(b0, space0)
{
    float2 _35_u_skRTFlip : packoffset(c1024);
};

Texture2D<float4> t : register(t0, space0);
SamplerState _t_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    return t.SampleGrad(_t_sampler, _26, ddx(_26), ddy(_26) * _35_u_skRTFlip.yy);
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
