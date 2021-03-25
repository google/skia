#version 400
out vec4 sk_FragColor;
uniform vec4 colorWhite;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 _0_MakeTempVar;
    if (colorWhite.xy == colorWhite.zw) {
        _0_MakeTempVar = colorGreen;
    } else {
        _0_MakeTempVar = colorRed;
    }
    return _0_MakeTempVar;
}
