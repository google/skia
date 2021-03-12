#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_MakeTempVar;
    {
        vec4 _1_d = color * 0.75;
        _0_MakeTempVar = _1_d.xxxx;
    }
    sk_FragColor = _0_MakeTempVar;
}
