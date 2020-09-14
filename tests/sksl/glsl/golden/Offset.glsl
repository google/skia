
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    struct Test {
        layout (offset = 0) highp int x;
        layout (offset = 4) highp int y;
        highp int z;
    } t;
    t.x = 0;
    sk_FragColor.x = float(t.x);
}
