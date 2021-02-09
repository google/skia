
out vec4 sk_FragColor;
void main() {
    int x = 1;
    switch (x) {
        case 1:
            sk_FragColor = vec4(1.0);
            break;
        default:
            sk_FragColor = vec4(0.0);
    }
}
