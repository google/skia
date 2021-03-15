
out vec4 sk_FragColor;
uniform float a;
int b;
void main() {
    sk_FragColor.x = ldexp(a, b);
}
