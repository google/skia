
out vec4 sk_FragColor;
uniform float unknownInput;
void main() {
    float value = 0.0;
    switch (int(unknownInput)) {
        case 0:
            value = 0.0;
        case 1:
            value = 1.0;
    }
    sk_FragColor = vec4(value);
}
