
out vec4 sk_FragColor;
vec4 main() {
    float rgb[3];
    float a;
    rgb[0] = 0.0;
    rgb[1] = 1.0;
    rgb[2] = 0.0;
    a = 1.0;
    return vec4(rgb[0], rgb[1], rgb[2], a);
}
