#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _0_c = colorRed;
    {
        vec4 _1_d = colorGreen;
        _0_c = _1_d;
    }
    return _0_c;
}
