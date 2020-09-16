
out vec4 sk_FragColor;
void main() {
    int exp;
    float foo = frexp(0.5, exp);
    sk_FragColor = vec4(float(exp));
}
