
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp int value;
mediump vec4 switchy(highp int v) {
    switch (v) {
        case 0:
            return vec4(0.5);
    }
    return vec4(1.0);
}
void main() {
    sk_FragColor = switchy(value);
}
