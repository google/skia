cbuffer _UniformBuffer : register(b0, space0)
{
    float2 _13_ah : packoffset(c0);
    float2 _13_bh : packoffset(c0.z);
    float2 _13_af : packoffset(c1);
    float2 _13_bf : packoffset(c1.z);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float cross_length_2d_hh2h2(float2 _27, float2 _28)
{
    return determinant(float2x2(_27, _28));
}

float cross_length_2d_ff2f2(float2 _19, float2 _20)
{
    return determinant(float2x2(_19, _20));
}

void frag_main()
{
    float2 _41 = _13_ah;
    float2 _45 = _13_bh;
    sk_FragColor.x = cross_length_2d_hh2h2(_41, _45);
    float2 _52 = _13_af;
    float2 _56 = _13_bf;
    sk_FragColor.y = cross_length_2d_ff2f2(_52, _56);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
