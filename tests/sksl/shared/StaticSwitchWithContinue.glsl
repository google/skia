
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float result = 0.0;
    for (int x = 0;x <= 1; x++) {
        {
            result = abs(2.0);
            continue;
        }
    }
    return result == 2.0 ? colorGreen : colorRed;
}
