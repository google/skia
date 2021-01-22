#version 400
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    highp float x = sqrt(1.0);
    highp vec4 y = vec4(x);
    sk_FragColor = y;
}
