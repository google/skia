
out vec4 sk_FragColor;
struct S {
    vec2 v;
};
void initialize_vS(out S z[2]) {
    z[0].v = vec2(0.0, 1.0);
    z[1].v = vec2(2.0, 1.0);
}
vec4 main() {
    vec2 x[2];
    x[0] = vec2(0.0);
    x[1] = vec2(1.0, 0.0);
    vec2 y[2];
    y[0] = vec2(0.0, 1.0);
    y[1] = vec2(-1.0, 2.0);
    S z[2];
    initialize_vS(z);
    return vec4(x[0].x * x[0].y + z[0].v.x, x[1].x - x[1].y * z[0].v.y, (y[0].x / y[0].y) / z[1].v.x, y[1].x + y[1].y * z[1].v.y);
}
