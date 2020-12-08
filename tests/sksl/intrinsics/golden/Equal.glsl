
out vec4 sk_FragColor;
in vec4 a;
in vec4 b;
bvec4 c;
void main() {
    sk_FragColor.x = float(equal(a, b) == c ? 1 : 0);
}
