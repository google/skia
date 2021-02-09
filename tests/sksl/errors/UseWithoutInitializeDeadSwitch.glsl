
out vec4 sk_FragColor;
void dead_switch() {
    int x;
    switch (3) {
        case 0:
            x = 0;
        case 1:
            x = 1;
    }
    sk_FragColor = vec4(float(x));
}
