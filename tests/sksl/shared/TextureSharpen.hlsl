Texture2D<float4> s : register(t0, space0);
SamplerState _s_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _20 = s.SampleBias(_s_sampler, 0.0f.xx, -0.4749999940395355224609375f);
    float4 a = _20;
    float4 _27 = s.SampleBias(_s_sampler, 0.0f.xxx.xy / 0.0f.xxx.z, -0.4749999940395355224609375f);
    float4 b = _27;
    sk_FragColor = float4(_20.xy, _27.xy);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
