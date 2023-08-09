
out vec4 sk_FragColor;
uniform vec4 colorGreen;
float setGreen_vh4(inout vec4 c) {
    c.y = 1.0;
    return 0.0;
}
float setAlpha_vh4(inout vec4 c) {
    c.w = 1.0;
    return 0.0;
    return 0.0;
}
vec4 main() {
    vec4 color = vec4(0.0);
    return ((setGreen_vh4(color), setAlpha_vh4(color)), color);
}
