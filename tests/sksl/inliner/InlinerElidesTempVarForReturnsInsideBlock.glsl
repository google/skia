#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_c = color;
    {
        vec4 _1_d = _0_c * 0.75;
        _0_c = _1_d;
    }
    sk_FragColor = _0_c.xxxx;
}
