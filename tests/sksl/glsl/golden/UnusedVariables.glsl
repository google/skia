
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    highp float b = 2.0;
    highp float d = 3.0;
    b++;
    d++;
    sk_FragColor = vec4(b, b, d, d);
}
