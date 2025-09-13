cbuffer testBlockA : register(b1, space0)
{
    float2 _7_x : packoffset(c0);
};

cbuffer testBlockB : register(b2, space0)
{
    float2 _12_y : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = float4(_7_x, _12_y);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
