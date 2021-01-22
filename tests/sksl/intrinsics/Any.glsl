
out vec4 sk_FragColor;
bvec4 a;
void main() {
    sk_FragColor.x = float(any(a) ? 1 : 0);
}
