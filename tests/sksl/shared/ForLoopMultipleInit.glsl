
out vec4 sk_FragColor;
vec4 main() {
    vec4 result = vec4(0.0);
    for (int a = 0;
    int b = 0;
    a < 10 && b < 10; (++a , ++b)) {
        result.x += 1.0;
        result.y += 2.0;
    }
    return result;
}
