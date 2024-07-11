static float4 sk_FragColor;
static float defaultVarying;
static float linearVarying;
static float flatVarying;

struct SPIRV_Cross_Input
{
    float defaultVarying : TEXCOORD0;
    noperspective float linearVarying : TEXCOORD1;
    nointerpolation float flatVarying : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    sk_FragColor = float4(defaultVarying, linearVarying, flatVarying, 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    defaultVarying = stage_input.defaultVarying;
    linearVarying = stage_input.linearVarying;
    flatVarying = stage_input.flatVarying;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
