
layout (location = 1) out int id;
int fn_i() {
    return gl_VertexID;
}
void main() {
    id = fn_i();
}
