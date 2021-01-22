
out vec4 sk_FragColor;
in float a;
in float b;
in float c;
void main() {
    sk_FragColor.x = ((a) * (b) + (c));
}
