
out vec4 sk_FragColor;
in float a;
in float b;
in vec4 c;
in vec4 d;
void main() {
    sk_FragColor.x = atan(a);
    sk_FragColor.x = atan(a, b);
    sk_FragColor = atan(c);
    sk_FragColor = atan(c, d);
}
