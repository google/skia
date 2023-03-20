cbuffer sksl_synthetic_uniforms : register(b0, space0)
{
    float2 _13_u_skRTFlip : packoffset(c1024);
};


static bool gl_FrontFacing;
static float4 sk_FragColor;

struct SPIRV_Cross_Input
{
    bool gl_FrontFacing : SV_IsFrontFace;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool _25 = false;
    if (_13_u_skRTFlip.y > 0.0f)
    {
        _25 = !gl_FrontFacing;
    }
    else
    {
        _25 = gl_FrontFacing;
    }
    sk_FragColor = float(_25 ? 1 : (-1)).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FrontFacing = stage_input.gl_FrontFacing;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
