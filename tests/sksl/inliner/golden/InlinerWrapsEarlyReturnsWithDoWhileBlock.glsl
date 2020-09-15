
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
mediump vec4 returny(mediump vec4 c) {
    if (c.x > c.y) return c.xxxx;
    if (c.y > c.z) return c.yyyy;
    return c.zzzz;
}
void main() {
    sk_FragColor = returny(color);
}
