#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_MakeTempVar;
    {
        _0_MakeTempVar = color.xxxx;
    }
    sk_FragColor = _0_MakeTempVar;

}
