
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    vec4 c = colorGreen;
    float _0_x = c.y;
    _0_x *= _0_x;
    c.y = _0_x;
    return c;
}
