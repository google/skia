
out vec4 sk_FragColor;
void main() {
    vec2 x[2];
    x[0] = vec2(1.0);
    x[1] = vec2(2.0);
    vec2[2] y;
    y[0] = vec2(3.0);
    y[1] = vec2(4.0);
    sk_FragColor = vec4(x[0], y[1]);
}
