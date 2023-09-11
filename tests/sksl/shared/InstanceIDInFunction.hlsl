static int gl_InstanceIndex;
static int id;

struct SPIRV_Cross_Input
{
    uint gl_InstanceIndex : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    int id : TEXCOORD1;
};

int fn_i()
{
    return gl_InstanceIndex;
}

void vert_main()
{
    id = fn_i();
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_InstanceIndex = int(stage_input.gl_InstanceIndex);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.id = id;
    return stage_output;
}
