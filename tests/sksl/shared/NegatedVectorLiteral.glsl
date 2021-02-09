
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_float() {
    float one = 1.0;
    float two = 2.0;
    vec4 result;
    result.x = float(vec4(-1.0) == -vec4(-vec2(-1.0), vec2(1.0)) ? 1 : 0);
    result.y = float(vec4(1.0) != -vec4(1.0) ? 1 : 0);
    result.z = float(-vec4(two) == vec4(-two, vec3(-two)) ? 1 : 0);
    result.w = float(-vec2(-one, one + one) == -vec2(one - two, two) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
bool test_int() {
    int one = 1;
    int two = 2;
    ivec4 result;
    result.x = int(ivec4(-1) == -ivec4(-ivec2(-1), ivec2(1)) ? 1 : 0);
    result.y = int(ivec4(1) != -ivec4(1) ? 1 : 0);
    result.z = int(-ivec4(two) == ivec4(-two, ivec3(-two)) ? 1 : 0);
    result.w = int(-ivec2(-one, one + one) == -ivec2(one - two, two) ? 1 : 0);
    return bool(((result.x * result.y) * result.z) * result.w);
}
vec4 main() {
    return test_float() && test_int() ? colorGreen : colorRed;
}
