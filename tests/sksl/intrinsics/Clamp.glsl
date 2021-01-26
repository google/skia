
out vec4 sk_FragColor;
in float a;
in float b;
in float c;
in int d;
in int e;
in int f;
void main() {
    sk_FragColor.x = clamp(a, b, c);
    sk_FragColor.x = float(clamp(d, e, f));
}
