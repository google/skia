
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = dFdx(1.0);
}
