
out vec4 sk_FragColor;
in float a;
in float b;
in vec4 c;
in vec4 d;
void main() {
    sk_FragColor.x = mod(a, b);
    sk_FragColor = mod(c, b);
    sk_FragColor = mod(c, d);
}
