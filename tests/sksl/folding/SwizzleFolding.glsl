
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool _2_ok = true;
    _2_ok = _2_ok && colorGreen != colorRed;
    return _2_ok ? colorGreen : colorRed;
}
