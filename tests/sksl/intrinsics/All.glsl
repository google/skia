
out vec4 sk_FragColor;
bvec4 a;
void main() {
    sk_FragColor.x = float(all(a) ? 1 : 0);
}
