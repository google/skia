
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform testBlock {
    mediump float x;
    float[2] y;
    layout (binding = 12) mediump mat3x2 z;
    bool w;
};
void main() {
    sk_FragColor = vec4(x, y[0], y[1], 0.0);
}
