static float4 gl_Position;
static int gl_VertexIndex;
static int gl_InstanceIndex;
static float2 vcoord_Stage0;

struct SPIRV_Cross_Input
{
    uint gl_VertexIndex : SV_VertexID;
    uint gl_InstanceIndex : SV_InstanceID;
};

struct SPIRV_Cross_Output
{
    noperspective float2 vcoord_Stage0 : TEXCOORD1;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    int _22 = gl_InstanceIndex % 200;
    int x = _22;
    int _25 = gl_InstanceIndex / 200;
    int y = _25;
    int _31 = (gl_InstanceIndex * 929) % 17;
    int ileft = _31;
    int _40 = (_31 + 1) + ((gl_InstanceIndex * 1637) % (17 - _31));
    int iright = _40;
    int _45 = (gl_InstanceIndex * 313) % 17;
    int itop = _45;
    int _53 = (_45 + 1) + ((gl_InstanceIndex * 1901) % (17 - _45));
    int ibot = _53;
    float outset = 0.03125f;
    float _63 = 0.0f;
    if (0 == ((_22 + _25) % 2))
    {
        _63 = -0.03125f;
    }
    else
    {
        _63 = 0.03125f;
    }
    outset = _63;
    float _73 = (float(_31) * 0.0625f) - _63;
    float l = _73;
    float _77 = (float(_40) * 0.0625f) + _63;
    float r = _77;
    float t = (float(_45) * 0.0625f) - _63;
    float b = (float(_53) * 0.0625f) + _63;
    float _92 = 0.0f;
    if (0 == (gl_VertexIndex % 2))
    {
        _92 = _73;
    }
    else
    {
        _92 = _77;
    }
    float2 vertexpos = 0.0f.xx;
    vertexpos.x = float(_22) + _92;
    float _104 = 0.0f;
    if (0 == (gl_VertexIndex / 2))
    {
        _104 = t;
    }
    else
    {
        _104 = b;
    }
    vertexpos.y = float(y) + _104;
    vcoord_Stage0.x = float((0 == (gl_VertexIndex % 2)) ? (-1) : 1);
    vcoord_Stage0.y = float((0 == (gl_VertexIndex / 2)) ? (-1) : 1);
    gl_Position = float4(vertexpos.x, vertexpos.y, 0.0f, 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_VertexIndex = int(stage_input.gl_VertexIndex);
    gl_InstanceIndex = int(stage_input.gl_InstanceIndex);
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vcoord_Stage0 = vcoord_Stage0;
    return stage_output;
}
