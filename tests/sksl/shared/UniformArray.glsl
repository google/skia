
out vec4 sk_FragColor;
uniform float testArray[5];
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    for (int index = 0;index < 5; ++index) {
        if (testArray[index] != float(index + 1)) {
            return colorRed;
        }
    }
    return colorGreen;
}
