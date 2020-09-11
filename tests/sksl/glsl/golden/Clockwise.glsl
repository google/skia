
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(float(gl_FrontFacing ? 1 : -1));
}
