
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
    vec4 _0_result;
    _0_result.x = 1.0;
    _0_result.y = 1.0;
    _0_result.z = 1.0;
    _0_result.w = 1.0;
    return bool(((_0_result.x * _0_result.y) * _0_result.z) * _0_result.w) && test_int() ? colorGreen : colorRed;

}
