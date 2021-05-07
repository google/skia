
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 b = vec4(1.0);
    vec4 c = vec4(1.0, 1.0, 1.0, 1.0);
    if (b != c) {
        return colorRed;
    } else {
        return colorGreen;
    }
}
