
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
    {
        float _3_five = 5.0;
        _0_ok = _0_ok && mat3(1.0, 2.0, 3.0, 4.0, _3_five, 6.0, 7.0, 8.0, 9.0)[1] == vec3(4.0, _3_five, 6.0);
    }
    {
        float _4_num = 6.0;
        _0_ok = _0_ok && mat3(1.0, 2.0, 3.0, 4.0, 5.0, _4_num++, 7.0, 8.0, 9.0)[0] == vec3(1.0, 2.0, 3.0);
        _0_ok = _0_ok && mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, _4_num++, 8.0, 9.0)[1] == vec3(4.0, 5.0, 6.0);
        _0_ok = _0_ok && mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, _4_num++, 9.0)[2] == vec3(7.0, 8.0, 9.0);
    }
    {
        _0_ok = _0_ok && mat4(mat3(testMatrix2x2))[0] == vec4(1.0, 2.0, 0.0, 0.0);
        _0_ok = _0_ok && mat4(mat3(testMatrix2x2))[1] == vec4(3.0, 4.0, 0.0, 0.0);
    }
    return _0_ok ? colorGreen : colorRed;
}
