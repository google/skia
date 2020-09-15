
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
bool shouldLoop(mediump vec4 v) {
    return v.x < 0.5;
}
void main() {
    sk_FragColor = vec4(0.0);
    do {
        sk_FragColor += vec4(0.125);
    } while (shouldLoop(sk_FragColor));
}
