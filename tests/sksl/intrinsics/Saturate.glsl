
out vec4 sk_FragColor;
uniform float a;
void main() {
    sk_FragColor.x = clamp(a, 0.0, 1.0);
}
