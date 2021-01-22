
out vec4 sk_FragColor;
in float a;
in vec4 b;
void main() {
    sk_FragColor.x = length(a);
    sk_FragColor.x = length(b);
}
