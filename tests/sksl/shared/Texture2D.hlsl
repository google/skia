Texture2D<float4> tex : register(t0, space0);
SamplerState _tex_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _19 = tex.Sample(_tex_sampler, 0.0f.xx);
    float4 a = _19;
    float4 _25 = tex.Sample(_tex_sampler, 0.0f.xxx.xy / 0.0f.xxx.z);
    float4 b = _25;
    sk_FragColor = float4(_19.xy, _25.zw);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
