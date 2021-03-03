
noperspective layout (location = 1) out vec2 vcoord_Stage0;
void main() {
    int x = gl_InstanceID % 200;
    int y = gl_InstanceID / 200;
    int ileft = (gl_InstanceID * 929) % 17;
    int iright = (ileft + 1) + (gl_InstanceID * 1637) % (17 - ileft);
    int itop = (gl_InstanceID * 313) % 17;
    int ibot = (itop + 1) + (gl_InstanceID * 1901) % (17 - itop);
    float outset = 0.03125;
    outset = 0 == (x + y) % 2 ? -outset : outset;
    float l = float(ileft) / 16.0 - outset;
    float r = float(iright) / 16.0 + outset;
    float t = float(itop) / 16.0 - outset;
    float b = float(ibot) / 16.0 + outset;
    vec2 vertexpos;
    vertexpos.x = float(x) + (0 == gl_VertexID % 2 ? l : r);
    vertexpos.y = float(y) + (0 == gl_VertexID / 2 ? t : b);
    vcoord_Stage0.x = float(0 == gl_VertexID % 2 ? -1 : 1);
    vcoord_Stage0.y = float(0 == gl_VertexID / 2 ? -1 : 1);
    gl_Position = vec4(vertexpos.x, vertexpos.y, 0.0, 1.0);
}
