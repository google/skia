cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float4x4 _11_colorXform : packoffset(c0);
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
    float4 _20 = s.Sample(_s_sampler, 1.0f.xx);
    float4 tmpColor = _20;
    float4 _53 = 0.0f.xxxx;
    if (((any(bool4(_11_colorXform[0].x != float4(1.0f, 0.0f, 0.0f, 0.0f).x, _11_colorXform[0].y != float4(1.0f, 0.0f, 0.0f, 0.0f).y, _11_colorXform[0].z != float4(1.0f, 0.0f, 0.0f, 0.0f).z, _11_colorXform[0].w != float4(1.0f, 0.0f, 0.0f, 0.0f).w)) || any(bool4(_11_colorXform[1].x != float4(0.0f, 1.0f, 0.0f, 0.0f).x, _11_colorXform[1].y != float4(0.0f, 1.0f, 0.0f, 0.0f).y, _11_colorXform[1].z != float4(0.0f, 1.0f, 0.0f, 0.0f).z, _11_colorXform[1].w != float4(0.0f, 1.0f, 0.0f, 0.0f).w))) || any(bool4(_11_colorXform[2].x != float4(0.0f, 0.0f, 1.0f, 0.0f).x, _11_colorXform[2].y != float4(0.0f, 0.0f, 1.0f, 0.0f).y, _11_colorXform[2].z != float4(0.0f, 0.0f, 1.0f, 0.0f).z, _11_colorXform[2].w != float4(0.0f, 0.0f, 1.0f, 0.0f).w))) || any(bool4(_11_colorXform[3].x != float4(0.0f, 0.0f, 0.0f, 1.0f).x, _11_colorXform[3].y != float4(0.0f, 0.0f, 0.0f, 1.0f).y, _11_colorXform[3].z != float4(0.0f, 0.0f, 0.0f, 1.0f).z, _11_colorXform[3].w != float4(0.0f, 0.0f, 0.0f, 1.0f).w)))
    {
        float _69 = _20.w;
        _53 = float4(clamp(mul(float4(_20.xyz, 1.0f), _11_colorXform).xyz, 0.0f.xxx, _69.xxx), _69);
    }
    else
    {
        _53 = _20;
    }
    sk_FragColor = _53;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
