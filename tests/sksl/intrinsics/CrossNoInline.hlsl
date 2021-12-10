cbuffer _UniformBuffer : register(b0, space0)
{
    float2 _12_ah : packoffset(c0);
    float2 _12_bh : packoffset(c0.z);
    float2 _12_af : packoffset(c1);
    float2 _12_bf : packoffset(c1.z);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float cross_hh2h2(float2 _18, float2 _19)
{
    return (_18.x * _19.y) - (_18.y * _19.x);
}

float cross_ff2f2(float2 _32, float2 _33)
{
    return (_32.x * _33.y) - (_32.y * _33.x);
}

void frag_main()
{
    float2 _54 = _12_ah;
    float2 _58 = _12_bh;
    sk_FragColor.x = cross_hh2h2(_54, _58);
    float2 _65 = _12_af;
    float2 _69 = _12_bf;
    sk_FragColor.y = cross_ff2f2(_65, _69);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
