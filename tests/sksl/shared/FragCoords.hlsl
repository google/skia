cbuffer sksl_synthetic_uniforms : register(b0, space0)
{
    float2 _15_u_skRTFlip : packoffset(c1024);
};


static float4 gl_FragCoord;
static float4 sk_FragColor;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    float2 _39 = float4(gl_FragCoord.x, _15_u_skRTFlip.x + (_15_u_skRTFlip.y * gl_FragCoord.y), gl_FragCoord.z, gl_FragCoord.w).xy;
    sk_FragColor = float4(_39.x, _39.y, sk_FragColor.z, sk_FragColor.w);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
