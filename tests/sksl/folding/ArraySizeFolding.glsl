
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool check_array_is_int_2_bi(int x[2]) {
    return true;
}
bool check_array_is_float_3_bf(float x[3]) {
    return true;
}
bool test_param_bff(float a[3], float b[3]) {
    return true && check_array_is_float_3_bf(b);
}
vec4 main() {
    float f[3];
    float g[3];
    int _2_b[2];
    int _3_c[2];
    int _4_d[2];
    return ((check_array_is_int_2_bi(_2_b) && check_array_is_int_2_bi(_3_c)) && check_array_is_int_2_bi(_4_d)) && test_param_bff(f, g) ? colorGreen : colorRed;
}
