
layout (location = 1) out int id;
int fn_i() {
    return gl_InstanceID;
}
void main() {
    id = fn_i();
}
