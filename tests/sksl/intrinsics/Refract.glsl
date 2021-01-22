
out vec4 sk_FragColor;
in float a;
in float b;
in float c;
in vec4 d;
in vec4 e;
void main() {
    sk_FragColor.x = refract(a, b, c);
    sk_FragColor = refract(d, e, c);
}
