cbuffer testBlock : register(b0, space0)
{
    float _7_x : packoffset(c0);
    int _7_w : packoffset(c0.y);
    float _7_y[2] : packoffset(c1);
    row_major float3x3 _7_z : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = float4(_7_x, _7_y[0], _7_y[1], 0.0f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
