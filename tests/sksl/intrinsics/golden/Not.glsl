
out vec4 sk_FragColor;
bvec4 a;
void main() {
    sk_FragColor.x = float(not(a).x ? 1 : 0);
}
