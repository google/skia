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
    int x = gl_InstanceIndex % 200;
    int y = gl_InstanceIndex / 200;
    int ileft = (gl_InstanceIndex * 929) % 17;
    int iright = (ileft + 1) + ((gl_InstanceIndex * 1637) % (17 - ileft));
    int itop = (gl_InstanceIndex * 313) % 17;
    int ibot = (itop + 1) + ((gl_InstanceIndex * 1901) % (17 - itop));
    float outset = 0.03125f;
    float _69 = 0.0f;
    if (0 == ((x + y) % 2))
    {
        _69 = -outset;
    }
    else
    {
        _69 = outset;
    }
    outset = _69;
    float l = (float(ileft) * 0.0625f) - outset;
    float r = (float(iright) * 0.0625f) + outset;
    float t = (float(itop) * 0.0625f) - outset;
    float b = (float(ibot) * 0.0625f) + outset;
    float _109 = 0.0f;
    if (0 == (gl_VertexIndex % 2))
    {
        _109 = l;
    }
    else
    {
        _109 = r;
    }
    float2 vertexpos = 0.0f.xx;
    vertexpos.x = float(x) + _109;
    float _123 = 0.0f;
    if (0 == (gl_VertexIndex / 2))
    {
        _123 = t;
    }
    else
    {
        _123 = b;
    }
    vertexpos.y = float(y) + _123;
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
