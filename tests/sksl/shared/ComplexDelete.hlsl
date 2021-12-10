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
    float4 tmpColor = s.Sample(_s_sampler, 1.0f.xx);
    float4x4 _33 = float4x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float4 _41 = _33[0];
    float4 _45 = _33[1];
    float4 _50 = _33[2];
    float4 _55 = _33[3];
    float4 _59 = 0.0f.xxxx;
    if (((any(bool4(_14_colorXform[0].x != _41.x, _14_colorXform[0].y != _41.y, _14_colorXform[0].z != _41.z, _14_colorXform[0].w != _41.w)) || any(bool4(_14_colorXform[1].x != _45.x, _14_colorXform[1].y != _45.y, _14_colorXform[1].z != _45.z, _14_colorXform[1].w != _45.w))) || any(bool4(_14_colorXform[2].x != _50.x, _14_colorXform[2].y != _50.y, _14_colorXform[2].z != _50.z, _14_colorXform[2].w != _50.w))) || any(bool4(_14_colorXform[3].x != _55.x, _14_colorXform[3].y != _55.y, _14_colorXform[3].z != _55.z, _14_colorXform[3].w != _55.w)))
    {
        _59 = float4(clamp(mul(float4(tmpColor.xyz, 1.0f), _14_colorXform).xyz, 0.0f.xxx, tmpColor.w.xxx), tmpColor.w);
    }
    else
    {
        _59 = tmpColor;
    }
    sk_FragColor = _59;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
