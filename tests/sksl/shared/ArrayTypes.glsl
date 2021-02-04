
out vec4 sk_FragColor;
vec4 main() {
    vec2 x[2];
    x[0] = vec2(0.0, 0.0);
    x[1] = vec2(1.0, 0.0);
    vec2 y[2];
    y[0] = vec2(0.0, 1.0);
    y[1] = vec2(-1.0, 2.0);
    return vec4(x[0].x * x[0].y, x[1].x - x[1].y, y[0].x / y[0].y, y[1].x + y[1].y);
}
