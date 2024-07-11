static float defaultVarying;
static float linearVarying;
static float flatVarying;

struct SPIRV_Cross_Output
{
    float defaultVarying : TEXCOORD0;
    noperspective float linearVarying : TEXCOORD1;
    nointerpolation float flatVarying : TEXCOORD2;
};

void vert_main()
{
    defaultVarying = 1.0f;
    linearVarying = 2.0f;
    flatVarying = 3.0f;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.defaultVarying = defaultVarying;
    stage_output.linearVarying = linearVarying;
    stage_output.flatVarying = flatVarying;
    return stage_output;
}
