
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform testBlock {
    highp float x;
} test[2];
void main() {
    sk_FragColor = vec4(test[1].x);
}
