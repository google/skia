
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int counter = 0;
    const int increment = 1;
    for (int i = 0;i < 10; i += increment) {
        const int increment = 10;
        if (i == 0) {
            continue;
        }
        counter += increment;
    }
    return counter == 90 ? colorGreen : colorRed;
}
