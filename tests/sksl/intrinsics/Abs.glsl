
out vec4 sk_FragColor;
in float a;
in int b;
void main() {
    sk_FragColor.x = abs(a);
    sk_FragColor.x = float(abs(b));
}
