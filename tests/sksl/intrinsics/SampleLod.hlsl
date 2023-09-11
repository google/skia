Texture2D<float4> t : register(t0, space0);
SamplerState _t_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _16 = t.SampleLevel(_t_sampler, 0.0f.xx, 0.0f);
    float4 c = _16;
    sk_FragColor = _16 * t.SampleLevel(_t_sampler, 1.0f.xxx.xy / 1.0f.xxx.z, 0.0f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
