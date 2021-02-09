
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 c = abs(vec4(1.0));
    if (vec4(1.0) != c) {
        return colorRed;
    } else {
        return colorGreen;
    }
}
