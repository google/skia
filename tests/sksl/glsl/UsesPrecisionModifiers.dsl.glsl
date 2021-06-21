#version 400
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    mediump float x = 0.75;
    highp float y = 1.0;
    x++;
    y++;
    sk_FragColor.xy = vec2(x, y);
}
