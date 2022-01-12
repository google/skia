
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
    int _3_b[2];
    int _4_c[2];
    int _5_d[2];
    int _6_e[2];
    int _7_f[2];
    return ((((check_array_is_int_2_bi(_3_b) && check_array_is_int_2_bi(_4_c)) && check_array_is_int_2_bi(_5_d)) && check_array_is_int_2_bi(_6_e)) && check_array_is_int_2_bi(_7_f)) && test_param_bff(f, g) ? colorGreen : colorRed;
}
