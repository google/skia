
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
vec4 main() {
    bool _0_ok = true;
    _0_ok = _0_ok && mat3(unknownInput) == mat3(mat2(1.0));
    _0_ok = _0_ok && mat3(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, unknownInput) == mat3(mat2(9.0));
    return _0_ok ? colorGreen : colorRed;
}
