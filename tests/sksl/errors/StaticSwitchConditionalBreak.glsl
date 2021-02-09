
out vec4 sk_FragColor;
void conditional_break_in_static_switch() {
    int x = 1;
    switch (x) {
        case 1:
            sk_FragColor = vec4(1.0);
            if (sqrt(0.0) < sqrt(1.0)) break;
        default:
            sk_FragColor = vec4(0.0);
    }
}
