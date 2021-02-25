
out vec4 sk_FragColor;
void main() {
    float x;
    switch (1) {
        case 0:
            x = 0.0;
            break;
        default:
            x = 1.0;
            discard;
    }
    sk_FragColor = vec4(x);
}
