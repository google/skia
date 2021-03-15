
out vec4 sk_FragColor;
uniform float a;
uniform vec4 b;
void main() {
    sk_FragColor.x = normalize(a);
    sk_FragColor = normalize(b);
}
