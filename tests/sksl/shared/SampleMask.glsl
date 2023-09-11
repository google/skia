
out vec4 sk_FragColor;
void main() {
    gl_SampleMask[0] = int(4294967295u);
    sk_FragColor = vec4(float(uint(gl_SampleMaskIn[0]))) * 0.00390625;
}
