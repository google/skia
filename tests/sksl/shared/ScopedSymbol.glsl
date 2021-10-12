
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
int glob;
struct S {
    int i;
};
bool block_variable_hides_global_variable_b() {
    return glob == 2;
}
bool local_variable_hides_struct_b() {
    bool S = true;
    return S;
}
bool local_struct_variable_hides_struct_type_b() {
    S S = S(1);
    return S.i == 1;
}
bool local_variable_hides_global_variable_b() {
    int glob = 1;
    return glob == 1;
}
vec4 main() {
    glob = 2;
    bool _0_var = true;
    return (((_0_var && block_variable_hides_global_variable_b()) && local_variable_hides_struct_b()) && local_struct_variable_hides_struct_type_b()) && local_variable_hides_global_variable_b() ? colorGreen : colorRed;
}
