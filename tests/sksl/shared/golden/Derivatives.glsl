#version 400
#extension GL_OES_standard_derivatives : require
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
void main() {
    sk_FragColor.x = dFdx(1.0);
}
