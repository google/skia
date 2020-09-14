
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
const mediump float test[] = float[](1.0, 2.0, 3.0, 4.0);
void main() {
    sk_FragColor = vec4(test[0], test[1], test[2], test[3]);
}
