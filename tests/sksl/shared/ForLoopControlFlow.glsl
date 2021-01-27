
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 main() {
    vec4 x = colorWhite;
    for (float r = -5.0;r < 5.0; r += 1.0) {
        x.x = clamp(r, 0.0, 1.0);
        if (x.x == 0.0) break;
    }
    for (float b = 5.0;b >= 0.0; b -= 1.0) {
        x.z = b;
        if (x.w == 1.0) continue;
        x.y = 0.0;
    }
    return x;
}
