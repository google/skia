#version 400
out vec4 sk_FragColor;
uniform vec4 color;
vec4 MakeTempVar(vec4 c) {
    if (c.x < c.y) {
        return c.xxxx;
    } else {
        return c.yyyy;
    }
}
void main() {
    sk_FragColor = MakeTempVar(color);
}
