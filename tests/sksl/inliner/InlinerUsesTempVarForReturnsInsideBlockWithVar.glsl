#version 400
out vec4 sk_FragColor;
uniform vec4 color;
vec4 MakeTempVar(vec4 c) {
    {
        vec4 d = c * 0.75;
        return d.xxxx;
    }
}
void main() {
    sk_FragColor = MakeTempVar(color);
}
