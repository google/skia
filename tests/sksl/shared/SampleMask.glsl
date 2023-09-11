
out vec4 sk_FragColor;
vec4 samplemaskin_as_color_h4() {
    return vec4(float(uint(gl_SampleMaskIn[0])));
}
void clear_samplemask_v() {
    gl_SampleMask[0] = int(0u);
}
void reset_samplemask_v() {
    gl_SampleMask[0] = int(uint(gl_SampleMaskIn[0]));
}
void main() {
    clear_samplemask_v();
    reset_samplemask_v();
    gl_SampleMask[0] = int(4294967295u);
    sk_FragColor = samplemaskin_as_color_h4() * 0.00390625;
}
