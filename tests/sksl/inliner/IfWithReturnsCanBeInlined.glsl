
out vec4 sk_FragColor;
uniform vec4 color;
vec4 branchy(vec4 c) {
    if (c.z == c.w) return c.yyyy; else return c.zzzz;
}
void main() {
    sk_FragColor = branchy(color);
}
