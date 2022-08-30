
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool check_array_is_int_2_bi(int x[2]) {
    return true;
}
bool test_b() {
    int a[2];
    int b[2];
    int c[2];
    int d[2];
    int e[2];
    int f[2];
    return ((((check_array_is_int_2_bi(a) && check_array_is_int_2_bi(b)) && check_array_is_int_2_bi(c)) && check_array_is_int_2_bi(d)) && check_array_is_int_2_bi(e)) && check_array_is_int_2_bi(f);
}
bool check_array_is_float_3_bf(float x[3]) {
    return true;
}
bool test_param_bff(float a[3], float b[3]) {
    return check_array_is_float_3_bf(a) && check_array_is_float_3_bf(b);
}
vec4 main() {
    float f[3];
    float g[3];
    return test_b() && test_param_bff(f, g) ? colorGreen : colorRed;
}
