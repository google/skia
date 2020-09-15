
out vec4 sk_FragColor;
uniform vec4 color;
vec4 returny(vec4 c) {
    if (c.x > c.y) return c.xxxx;
    if (c.y > c.z) return c.yyyy;
    return c.zzzz;
}
void main() {
    sk_FragColor = returny(color);
}
