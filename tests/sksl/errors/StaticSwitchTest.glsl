
out vec4 sk_FragColor;
void non_constant_test_in_static_switch() {
    int x = int(sqrt(1.0));
    switch (x) {
        case 1:
            sk_FragColor = vec4(1.0);
            break;
        default:
            sk_FragColor = vec4(0.0);
    }
}
