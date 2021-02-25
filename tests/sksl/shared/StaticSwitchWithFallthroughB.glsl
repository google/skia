
out vec4 sk_FragColor;
void main() {
    float x = 0.0;
    switch (1) {
        case 0:
            x = 0.0;
        case 1:
            x = 1.0;
    }
    sk_FragColor = vec4(x);
}
