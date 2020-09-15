
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp int value;
mediump vec4 loopy(highp int v) {
    for (highp int x = 0;x < 5; ++x) {
        if (x == v) return vec4(0.5);
    }
    return vec4(1.0);
}
void main() {
    sk_FragColor = loopy(value);
}
