RWTexture2D<unorm float4> aTexture : register(u1, space0);
Texture2D<float4> aSampledTexture : register(t2, space0);
SamplerState _aSampledTexture_sampler : register(s2, space0);

static float4 sk_FragColor;
static float2 c;

struct SPIRV_Cross_Input
{
    float2 c : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 helpers_helper_h4ZT(Texture2D<float4> _20, SamplerState __20_sampler, RWTexture2D<unorm float4> _21)
{
    return _20.Sample(__20_sampler, c);
}

float4 helper_h4TZ(RWTexture2D<unorm float4> _27, Texture2D<float4> _28, SamplerState __28_sampler)
{
    return helpers_helper_h4ZT(_28, __28_sampler, _27);
}

void frag_main()
{
    sk_FragColor = helper_h4TZ(aTexture, aSampledTexture, _aSampledTexture_sampler);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    c = stage_input.c;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
