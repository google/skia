
out vec4 sk_FragColor;
float a;
int b;
void main() {
    sk_FragColor.x = ldexp(a, b);
}
