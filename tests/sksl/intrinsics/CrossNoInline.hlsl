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

float cross_length_2d_hh2h2(float2 _26, float2 _27)
{
    return determinant(float2x2(_26, _27));
}

float cross_length_2d_ff2f2(float2 _18, float2 _19)
{
    return determinant(float2x2(_18, _19));
}

void frag_main()
{
    float2 _41 = _12_ah;
    float2 _45 = _12_bh;
    sk_FragColor.x = cross_length_2d_hh2h2(_41, _45);
    float2 _52 = _12_af;
    float2 _56 = _12_bf;
    sk_FragColor.y = cross_length_2d_ff2f2(_52, _56);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
