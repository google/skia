cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float4x4 _14_colorXform : packoffset(c0);
};

Texture2D<float4> s : register(t0, space0);
SamplerState _s_sampler : register(s0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float4 _23 = s.Sample(_s_sampler, 1.0f.xx);
    float4 tmpColor = _23;
    float4 _55 = 0.0f.xxxx;
    if (((any(bool4(_14_colorXform[0].x != float4(1.0f, 0.0f, 0.0f, 0.0f).x, _14_colorXform[0].y != float4(1.0f, 0.0f, 0.0f, 0.0f).y, _14_colorXform[0].z != float4(1.0f, 0.0f, 0.0f, 0.0f).z, _14_colorXform[0].w != float4(1.0f, 0.0f, 0.0f, 0.0f).w)) || any(bool4(_14_colorXform[1].x != float4(0.0f, 1.0f, 0.0f, 0.0f).x, _14_colorXform[1].y != float4(0.0f, 1.0f, 0.0f, 0.0f).y, _14_colorXform[1].z != float4(0.0f, 1.0f, 0.0f, 0.0f).z, _14_colorXform[1].w != float4(0.0f, 1.0f, 0.0f, 0.0f).w))) || any(bool4(_14_colorXform[2].x != float4(0.0f, 0.0f, 1.0f, 0.0f).x, _14_colorXform[2].y != float4(0.0f, 0.0f, 1.0f, 0.0f).y, _14_colorXform[2].z != float4(0.0f, 0.0f, 1.0f, 0.0f).z, _14_colorXform[2].w != float4(0.0f, 0.0f, 1.0f, 0.0f).w))) || any(bool4(_14_colorXform[3].x != float4(0.0f, 0.0f, 0.0f, 1.0f).x, _14_colorXform[3].y != float4(0.0f, 0.0f, 0.0f, 1.0f).y, _14_colorXform[3].z != float4(0.0f, 0.0f, 0.0f, 1.0f).z, _14_colorXform[3].w != float4(0.0f, 0.0f, 0.0f, 1.0f).w)))
    {
        float _71 = _23.w;
        _55 = float4(clamp(mul(float4(_23.xyz, 1.0f), _14_colorXform).xyz, 0.0f.xxx, _71.xxx), _71);
    }
    else
    {
        _55 = _23;
    }
    sk_FragColor = _55;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
