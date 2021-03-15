
out vec4 sk_FragColor;
uniform float a;
uniform float b;
void main() {
    sk_FragColor.x = pow(a, b);
}
