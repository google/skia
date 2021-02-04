
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_int() {
    ivec4 result;
    result.x = 1;
    result.y = 1;
    result.z = 1;
    result.w = 1;
    return bool(((result.x * result.y) * result.z) * result.w);
}
vec4 main() {
    vec4 _1_result;
    _1_result.x = 1.0;
    _1_result.y = 1.0;
    _1_result.z = 1.0;
    _1_result.w = 1.0;
    return bool(((_1_result.x * _1_result.y) * _1_result.z) * _1_result.w) && test_int() ? colorGreen : colorRed;

}
