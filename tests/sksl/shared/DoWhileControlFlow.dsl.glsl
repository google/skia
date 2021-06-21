
out vec4 sk_FragColor;
vec4 main() {
    vec4 x = vec4(1.0, 1.0, 1.0, 1.0);
    do {
        x.x -= 0.25;
        if (x.x <= 0.0) break;
    } while (x.w == 1.0);
    do {
        x.z -= 0.25;
        if (x.w == 1.0) continue;
        x.y = 0.0;
    } while (x.z > 0.0);
    return x;
}
