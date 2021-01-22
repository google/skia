
out vec4 sk_FragColor;
in float a;
int b;
void main() {
    sk_FragColor.x = ldexp(a, b);
}
