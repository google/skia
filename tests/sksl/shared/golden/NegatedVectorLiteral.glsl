
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = 1.0;
    sk_FragColor.y = float(vec4(1.0) == -vec4(1.0) ? 1 : 0);
    sk_FragColor.z = float(vec4(0.0) == -vec4(0.0) ? 1 : 0);
}
