
out vec4 sk_FragColor;
void main() {
    int i = int(sqrt(1.0));
    i * ivec4(i = 3).x;
    sk_FragColor.x = float(i);
}
