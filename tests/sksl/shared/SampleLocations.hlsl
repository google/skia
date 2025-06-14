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
    int _24 = gl_InstanceIndex % 200;
    int x = _24;
    int _27 = gl_InstanceIndex / 200;
    int y = _27;
    int _33 = (gl_InstanceIndex * 929) % 17;
    int ileft = _33;
    int _42 = (_33 + 1) + ((gl_InstanceIndex * 1637) % (17 - _33));
    int iright = _42;
    int _47 = (gl_InstanceIndex * 313) % 17;
    int itop = _47;
    int _55 = (_47 + 1) + ((gl_InstanceIndex * 1901) % (17 - _47));
    int ibot = _55;
    float outset = 0.03125f;
    float _65 = 0.0f;
    if (0 == ((_24 + _27) % 2))
    {
        _65 = -0.03125f;
    }
    else
    {
        _65 = 0.03125f;
    }
    outset = _65;
    float _75 = (float(_33) * 0.0625f) - _65;
    float l = _75;
    float _79 = (float(_42) * 0.0625f) + _65;
    float r = _79;
    float t = (float(_47) * 0.0625f) - _65;
    float b = (float(_55) * 0.0625f) + _65;
    float _94 = 0.0f;
    if (0 == (gl_VertexIndex % 2))
    {
        _94 = _75;
    }
    else
    {
        _94 = _79;
    }
    float2 vertexpos = 0.0f.xx;
    vertexpos.x = float(_24) + _94;
    float _106 = 0.0f;
    if (0 == (gl_VertexIndex / 2))
    {
        _106 = t;
    }
    else
    {
        _106 = b;
    }
    vertexpos.y = float(y) + _106;
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
