Texture2D<float4> test2D : register(t0, space0);
SamplerState _test2D_sampler : register(s0, space0);
Texture2D<float4> test2DRect : register(t1, space0);
SamplerState _test2DRect_sampler : register(s1, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = test2D.Sample(_test2D_sampler, 0.5f.xx);
    sk_FragColor = test2DRect.Sample(_test2DRect_sampler, 0.5f.xx);
    sk_FragColor = test2DRect.Sample(_test2DRect_sampler, 0.5f.xxx.xy / 0.5f.xxx.z);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
