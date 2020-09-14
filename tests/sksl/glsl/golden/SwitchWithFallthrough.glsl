
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    highp float x = 0.0;
    switch (int(sqrt(3.0))) {
        case 0:
            x = 0.0;
        case 1:
            x = 1.0;
    }
    sk_FragColor = vec4(x);
}
