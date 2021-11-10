
out vec4 sk_FragColor;
uniform mat2 testMatrix2x2;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
vec4 main() {
    bool _0_ok = true;
    _0_ok = _0_ok && mat3(unknownInput) == mat3(mat2(1.0));
    _0_ok = _0_ok && mat3(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, unknownInput) == mat3(mat2(9.0));
    _0_ok = _0_ok && vec4(testMatrix2x2) == vec4(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && mat2(5.0)[0] == vec2(5.0, 0.0);
    _0_ok = _0_ok && mat2(5.0)[1] == vec2(0.0, 5.0);
    _0_ok = _0_ok && mat2(5.0)[0].x == 5.0;
    _0_ok = _0_ok && mat2(5.0)[0].y == 0.0;
    _0_ok = _0_ok && mat2(5.0)[1].x == 0.0;
    _0_ok = _0_ok && mat2(5.0)[1].y == 5.0;
    const mat3 _1_m = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _0_ok = _0_ok && _1_m[0] == vec3(1.0, 2.0, 3.0);
    _0_ok = _0_ok && _1_m[1] == vec3(4.0, 5.0, 6.0);
    _0_ok = _0_ok && _1_m[2] == vec3(7.0, 8.0, 9.0);
    _0_ok = _0_ok && _1_m[0].x == 1.0;
    _0_ok = _0_ok && _1_m[0].y == 2.0;
    _0_ok = _0_ok && _1_m[0].z == 3.0;
    _0_ok = _0_ok && _1_m[1].x == 4.0;
    _0_ok = _0_ok && _1_m[1].y == 5.0;
    _0_ok = _0_ok && _1_m[1].z == 6.0;
    _0_ok = _0_ok && _1_m[2].x == 7.0;
    _0_ok = _0_ok && _1_m[2].y == 8.0;
    _0_ok = _0_ok && _1_m[2].z == 9.0;
    return _0_ok ? colorGreen : colorRed;
}
