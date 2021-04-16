
out vec4 sk_FragColor;
vec4 main() {
    vec4 x = vec4(1.0);
    do {
        continue;
        x.y = 0.0;
    } while (x.z <= 0.0);
    return x;
}
