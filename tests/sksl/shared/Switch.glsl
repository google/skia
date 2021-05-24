
out vec4 sk_FragColor;
uniform float unknownInput;
void main() {
    float value;
    switch (int(unknownInput)) {
        case 0:
            value = 0.0;
            break;
        case 1:
            value = 1.0;
            break;
        default:
            value = 2.0;
    }
    sk_FragColor = vec4(value);
}
