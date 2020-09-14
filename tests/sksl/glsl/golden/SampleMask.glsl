
precision mediump float;
precision mediump sampler2D;
void main() {
    gl_SampleMask[0] |= 8;
}
