
out vec4 sk_FragColor;
void main() {
    int i = int(sqrt(1.0));
    i * ivec2(ivec4(i = 3).x, 1).x;
    sk_FragColor.x = float(i);
}
