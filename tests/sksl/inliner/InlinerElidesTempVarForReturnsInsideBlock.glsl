#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _1_c = color;
    {
        vec4 _2_d = _1_c * 0.75;
        _1_c = _2_d;
    }
    sk_FragColor = _1_c.xxxx;

}
