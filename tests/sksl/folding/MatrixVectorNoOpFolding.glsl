
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform mat3 testMatrix3x3;
uniform vec4 testInputs;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test_mat3_vec3_b() {
    vec3 v;
    vec3 vv;
    v = testInputs.xyz;
    v = testInputs.xyz;
    vv = vec3(0.0);
    vv = vec3(0.0);
    return v == testInputs.xyz && vv == vec3(0.0, 0.0, 0.0);
}
bool test_mat4_vec4_b() {
    vec4 v;
    vec4 vv;
    v = testInputs;
    v = testInputs;
    vv = vec4(0.0);
    vv = vec4(0.0);
    return v == testInputs && vv == vec4(0.0, 0.0, 0.0, 0.0);
}
vec4 main() {
    vec2 _2_v;
    vec2 _3_vv;
    _2_v = testInputs.xy;
    _2_v = testInputs.xy;
    _3_vv = vec2(0.0);
    _3_vv = vec2(0.0);
    return ((_2_v == testInputs.xy && _3_vv == vec2(0.0, 0.0)) && test_mat3_vec3_b()) && test_mat4_vec4_b() ? colorGreen : colorRed;
}
