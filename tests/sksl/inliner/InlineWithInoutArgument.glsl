
out vec4 sk_FragColor;
void outParameter(inout float x) {
    x *= 2.0;
}
void main() {
    float x = 1.0;
    outParameter(x);
    sk_FragColor.x = x;
}
