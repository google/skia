
out vec4 sk_FragColor;
uniform int unknownInput;
void main() {
    float value;
    switch (unknownInput) {
        case 0:
            value = 0.0;
        case 1:
            value = 1.0;
        default:
            value = 2.0;
    }
    sk_FragColor = vec4(value);
}
