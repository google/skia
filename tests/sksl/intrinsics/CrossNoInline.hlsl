cbuffer _UniformBuffer : register(b0, space0)
{
    float2 _9_ah : packoffset(c0);
    float2 _9_bh : packoffset(c0.z);
    float2 _9_af : packoffset(c1);
    float2 _9_bf : packoffset(c1.z);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float cross_length_2d_hh2h2(float2 _23, float2 _24)
{
    return determinant(float2x2(_23, _24));
}

float cross_length_2d_ff2f2(float2 _15, float2 _16)
{
    return determinant(float2x2(_15, _16));
}

void frag_main()
{
    float2 _38 = _9_ah;
    float2 _42 = _9_bh;
    sk_FragColor.x = cross_length_2d_hh2h2(_38, _42);
    float2 _49 = _9_af;
    float2 _53 = _9_bf;
    sk_FragColor.y = cross_length_2d_ff2f2(_49, _53);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
