
out vec4 sk_FragColor;
in vec2 a;
in vec2 b;
void main() {
    float _0_cross;
    _0_cross = a.x * b.y - a.y * b.x;

    sk_FragColor.x = _0_cross;

}
