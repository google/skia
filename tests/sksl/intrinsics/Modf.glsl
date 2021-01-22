
out vec4 sk_FragColor;
in float a;
in float b;
void main() {
    sk_FragColor.x = modf(a, b);
}
