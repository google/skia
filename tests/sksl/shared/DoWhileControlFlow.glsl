
out vec4 sk_FragColor;
vec4 main() {
    float x = 1.0;
    do {
        if (x == 3.0) continue;
    } while (x == 2.0);
    return vec4(0.0, 1.0, 0.0, 1.0);
}
