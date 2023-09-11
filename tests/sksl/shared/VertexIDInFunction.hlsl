static int gl_VertexIndex;
static int id;

struct SPIRV_Cross_Input
{
    uint gl_VertexIndex : SV_VertexID;
};

struct SPIRV_Cross_Output
{
    int id : TEXCOORD1;
};

int fn_i()
{
    return gl_VertexIndex;
}

void vert_main()
{
    id = fn_i();
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexIndex = int(stage_input.gl_VertexIndex);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.id = id;
    return stage_output;
}
