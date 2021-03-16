#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_returny;
    if (color.x > color.y) _0_returny = color.xxxx;
    if (color.y > color.z) _0_returny = color.yyyy;
    _0_returny = color.zzzz;
    sk_FragColor = _0_returny;
}
