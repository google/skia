
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool _0_ok = true;
    mat4x3 _1_m43 = mat4x3(7.0);
    _0_ok = _0_ok && _1_m43 == mat4x3(7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0);
    return _0_ok ? colorGreen : colorRed;
}
