
out vec4 sk_FragColor;
float setGreen_vh(out float g) {
    g = 1.0;
    return 0.0;
}
float setAlpha_vh(out float a) {
    a = 1.0;
    return 0.0;
    return 0.0;
}
vec4 main() {
    float green;
    float alpha;
    return ((setGreen_vh(green), setAlpha_vh(alpha)), vec4(0.0, green, 0.0, alpha));
}
