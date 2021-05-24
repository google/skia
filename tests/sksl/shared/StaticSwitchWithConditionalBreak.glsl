
out vec4 sk_FragColor;
uniform float unknownInput;
void main() {
    float value = 0.0;
    switch (0) {
        case 0:
            value = 0.0;
            if (unknownInput == 2.0) break;
        case 1:
            value = 1.0;
    }
    sk_FragColor = vec4(value);
}
