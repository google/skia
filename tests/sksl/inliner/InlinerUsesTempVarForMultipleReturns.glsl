#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_MakeTempVar;
    if (color.x < color.y) {
        _0_MakeTempVar = color.xxxx;
    } else {
        _0_MakeTempVar = color.yyyy;
    }
    sk_FragColor = _0_MakeTempVar;
}
