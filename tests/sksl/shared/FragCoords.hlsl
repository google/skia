cbuffer sksl_synthetic_uniforms : register(b0, space0)
{
    float2 _12_u_skRTFlip : packoffset(c1024);
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
    float2 _34 = float4(gl_FragCoord.x, _12_u_skRTFlip.x + (_12_u_skRTFlip.y * gl_FragCoord.y), gl_FragCoord.zw).yx;
    sk_FragColor = float4(_34.x, _34.y, sk_FragColor.z, sk_FragColor.w);
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
