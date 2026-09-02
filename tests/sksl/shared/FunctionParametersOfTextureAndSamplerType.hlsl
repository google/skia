Texture2D<float4> aTexture : register(t1, space0);
Texture2D<float4> aSampledTexture : register(t2, space0);
SamplerState _aSampledTexture_sampler : register(s2, space0);
Texture2D<float4> aSecondSampledTexture : register(t3, space0);
SamplerState _aSecondSampledTexture_sampler : register(s3, space0);

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

float4 helpers_helper_h4ZT_aSampledTexture(Texture2D<float4> _26)
{
    return aSampledTexture.Sample(_aSampledTexture_sampler, c);
}

float4 helper_h4TZ_aSampledTexture(Texture2D<float4> _36)
{
    return helpers_helper_h4ZT_aSampledTexture(_36);
}

float4 helpers_helper_h4ZT_aSecondSampledTexture(Texture2D<float4> _31)
{
    return aSecondSampledTexture.Sample(_aSecondSampledTexture_sampler, c);
}

float4 helper_h4TZ_aSecondSampledTexture(Texture2D<float4> _39)
{
    return helpers_helper_h4ZT_aSecondSampledTexture(_39);
}

void frag_main()
{
    sk_FragColor = helper_h4TZ_aSampledTexture(aTexture);
    sk_FragColor = helper_h4TZ_aSecondSampledTexture(aTexture);
    sk_FragColor = helper_h4TZ_aSampledTexture(aTexture);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    c = stage_input.c;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
