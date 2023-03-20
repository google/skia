
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool test_matrix_b() {
    bool ok = true;
    float num = 0.0;
    ok = ok && mat2(1.0, 2.0, 3.0, ++num)[0] == vec2(1.0, 2.0);
    ok = ok && mat2(vec2(++num), 3.0, 4.0)[1] == vec2(3.0, 4.0);
    ok = ok && mat3(vec3(1.0), vec3(++num), vec3(0.0))[0] == vec3(1.0);
    ok = ok && mat3(vec3(1.0), vec3(++num), vec3(0.0))[2] == vec3(0.0);
    ok = ok && mat3(vec3(++num), vec3(1.0), vec3(0.0))[1] == vec3(1.0);
    ok = ok && mat3(1.0, 2.0, 3.0, 4.0, 5.0, ++num, 7.0, 8.0, 9.0)[0] == vec3(1.0, 2.0, 3.0);
    ok = ok && mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, num++, 8.0, 9.0)[1] == vec3(4.0, 5.0, 6.0);
    ok = ok && mat4(vec4(++num), vec4(1.0), vec4(2.0), vec4(3.0))[1] == vec4(1.0);
    ok = ok && mat4(vec4(1.0), vec4(++num), vec4(2.0), vec4(3.0))[2] == vec4(2.0);
    ok = ok && mat4(vec4(1.0), vec4(1.0), vec4(++num), vec4(3.0))[3] == vec4(3.0);
    ok = ok && mat4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, ++num, 16.0)[3].xy == vec2(13.0, 14.0);
    return ok && num == 11.0;
}
vec4 main() {
    bool _0_ok = true;
    float _1_num = 0.0;
    _0_ok = _0_ok && vec2(++_1_num, 0.0).y == 0.0;
    _0_ok = _0_ok && vec2(0.0, ++_1_num).x == 0.0;
    _0_ok = _0_ok && vec3(++_1_num, 1.0, 0.0).yz == vec2(1.0, 0.0);
    _0_ok = _0_ok && vec3(1.0, 0.0, ++_1_num).xy == vec2(1.0, 0.0);
    _0_ok = _0_ok && vec3(++_1_num, 1.0, 0.0).yz == vec2(1.0, 0.0);
    _0_ok = _0_ok && vec4(++_1_num, 1.0, 0.0, 0.0).yzw == vec3(1.0, 0.0, 0.0);
    _0_ok = _0_ok && vec4(1.0, ++_1_num, 1.0, 0.0).x == 1.0;
    _0_ok = _0_ok && vec4(1.0, 0.0, ++_1_num, 1.0).w == 1.0;
    _0_ok = _0_ok && vec4(1.0, 0.0, 1.0, ++_1_num).xyz == vec3(1.0, 0.0, 1.0);
    return (_0_ok && _1_num == 9.0) && test_matrix_b() ? colorGreen : colorRed;
}
